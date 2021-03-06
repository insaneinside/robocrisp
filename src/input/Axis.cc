#include <crisp/input/Axis.hh>
#include <cmath>

namespace crisp
{
  namespace input
  {

    template class MappedEventSource<Axis, int32_t, double>;

    Axis::Axis(Axis::Type _type, Axis::ID _id)
      : BaseType ( _id ),
        type ( _type ),
        mode ( _type ),
        m_last_raw_value ( 0 ),
	raw ( ),
	map_method ( MapMethod::NONE ),
	coefficients ( )
    {
    }

    Axis::Axis(RawConfig _raw, Axis::ID _id)
      : BaseType ( _id ),
        type ( Axis::Type::ABSOLUTE ),
        mode ( type ),
        m_last_raw_value ( 0 ),
	raw ( _raw ),
	map_method ( MapMethod::LINEAR ),
	coefficients ( )
    {
    }


    Axis::Axis(RawConfig _raw, Axis::ID _id, const std::initializer_list<Value>& _coefficients)
      : BaseType ( _id ),
        type ( Axis::Type::ABSOLUTE ),
        mode ( type ),
        m_last_raw_value ( 0 ),
	raw ( _raw ),
	map_method ( MapMethod::POLYNOMIAL ),
	coefficients ( )
    {
      set_coefficients(_coefficients);
    }

    Axis::Axis(Axis&& a)
      : BaseType ( std::move(a) ),
        type ( std::move(a.type) ),
        mode ( std::move(a.mode) ),
        m_last_raw_value ( 0 ),
	raw ( std::move(a.raw) ),
	map_method ( std::move(a.map_method) ),
	coefficients ( std::move(a.coefficients) )
    {
    }

    Axis::~Axis()
    {}

    void
    Axis::set_coefficients(const std::initializer_list<Value>& list)
    {
      map_method = MapMethod::POLYNOMIAL;

      /* Here we just need to copy the values in `list` to `coefficients` in
	 reverse order.

	 To achieve that, we _resize_ (not reserve!) our coefficients list to the
	 correct number of elements, and then use std::reverse_copy to overwrite
	 the contents with the user-provided values.
      */
      coefficients.resize(list.size());

      std::reverse_copy(list.begin(), list.end(),
			coefficients.begin());
    }

    template < typename _T >
    inline int8_t sign(_T x)
    { return x < 0 ? -1 : 1; }

    Axis::Value
    Axis::map(Axis::RawValue raw_value) const
    {
      Value x ( 0.0 );

      /* Now: here's _how_ we're going to deal with things.

	 The simplest case is raw.maximum == -raw.minimum and raw.neutral == 0; in
	 that case, the returned value is just

	 sign(raw_value) * std::abs((Value) raw_value / (Value) raw.maximum).

	 But it's the edge cases that are more fun.

         - When raw.neutral == raw.minimum, we'll interpret this as requiring an
	 output that never goes below zero:

	 std::abs((Value) (raw_value - raw.neutral) /
	 (Value) (raw.maximum - raw.neutral))

	 (and similarly for the case that raw.neutral == raw.maximum; we just
	 make it have a negative slope).

	 - When (raw.maximum - raw.neutral) != -(raw.minimum - raw.neutral)
	 (i.e., the neutral point is not centered between minimum and
	 maximum), we still want a constant slope.  The solution is to
	 normalize against the larger (not greater) of the two extrema:

	 if abs(maximum - neutral) > abs(minimum - neutral):
	 scale = abs(maximum - neutral)
	 else:
	 scale = abs(minimum - neutral)

	 and

	 output = (raw_value - neutral) / scale .

	 [Yes, that's probably Python.  Because I can.]
      */
      /* Pre-calcuate some things, and use implicit conversions to avoid ugly
	 `static_cast`s later on. */
      Value 
	max ( raw.maximum - raw.neutral ),
	min ( raw.minimum - raw.neutral ),
	val ( raw_value - raw.neutral );
      

      if ( raw.maximum == -raw.minimum || /* base case */
	   raw.minimum == raw.neutral )	/* first edge case */
	x = val / max;
      else if ( raw.maximum == raw.neutral ) /* first edge case ["...we just make
						it have a negative slope..."] */
	x = val / min;

      else if ( std::abs(raw.maximum - raw.neutral) != std::abs(raw.minimum - raw.neutral) )
	/* Second edge case. */
	x = val / std::max(std::abs(max), std::abs(min));

      else
	throw std::runtime_error("Unhandled edge case in Axis::map(RawValue)");

      /* Return the secondary mapping value based on this one. */
      return map(x);
    }

    Axis::Value
    Axis::map(Axis::Value x) const
    {
      Value out ( 0.0 );

      switch ( map_method )
	{
        case MapMethod::NONE:
	case MapMethod::LINEAR:
	  out = x;
	  break;

	case MapMethod::POLYNOMIAL:
	  for ( size_t i ( coefficients.size() ); i > 0; --i )
	    out += coefficients[i - 1] * std::pow(x, i - 1);
	  break;
	}

      return out;
    }

    void
    Axis::post(Axis::RawValue raw_value)
    {
      State state = { raw_value, 0 };

      /* Handle axis-type emulation. */
      if ( mode != type )
        switch ( mode )
          {
          case Type::RELATIVE:
            state.raw_value = raw_value - m_last_raw_value;
            m_last_raw_value = raw_value;
            break;

          case Type::ABSOLUTE:
            state.raw_value += m_last_raw_value;

            m_last_raw_value = state.raw_value;
            break;
          }

      if ( type == Type::ABSOLUTE || mode == Type::ABSOLUTE )
        /* Clip raw_value to [minimum, maximum]. */
        state.raw_value = std::min(std::max(state.raw_value, raw.minimum), raw.maximum);

      if ( map_method != MapMethod::NONE )
        state.value = map(state.raw_value);

      m_signal.emit(*this, state);
    }

  }
}

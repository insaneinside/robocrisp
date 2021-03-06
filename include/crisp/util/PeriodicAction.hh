#ifndef crisp_util_PeriodicAction_hh
#define crisp_util_PeriodicAction_hh 1

#include <functional>
#include <boost/system/error_code.hpp>
#include <memory>

namespace crisp
{
  namespace util
  {

    class PeriodicScheduleSlot;

    /** An action scheduled on a PeriodicScheduleSlot.  `PeriodicAction` is used to
        manage a scheduled action by rescheduling, cancelling, or temporarily
        disabling it.  */
    struct PeriodicAction : public std::enable_shared_from_this<PeriodicAction>
    {
      /** Function type for user callbacks.
       *
       * @param action The Action object associated with the callback.
       */
      typedef std::function<void(PeriodicAction& action)> Function;
	
#ifdef SWIG
      %immutable;
#endif
      PeriodicScheduleSlot* slot; /**< Pointer to the schedule slot that
                                     contains this action.  This is used by
                                     `pause`, `unpause`, and `cancel`. */

#ifdef SWIG
      %immutable;
#endif
      Function function;	/**< User-defined function to be called by the
				   slot. */

#ifdef SWIG
      %immutable;
#endif
      bool active;              /**< When `true`, the slot will continue to
                                   enqueue timer waits on behalf of the action. */

      PeriodicAction(PeriodicScheduleSlot* _slot,
                     Function _function);

      inline std::weak_ptr<PeriodicAction>
      get_pointer() { return shared_from_this(); }

      /** Handler invoked by the slot's timer when it expires or is cancelled.
       * This function calls the user function whenever called with an empty
       * error object.
       *
       * @param error An error code that specifies why the function was called.
       */
      void
      timer_expiry_handler(const boost::system::error_code& error);

      bool
      operator < (const PeriodicAction& action) const;

      bool
      operator == (const PeriodicAction& action) const;


      /** Unset the `active` flag, preventing the action's associated slot from
          enqueuing timer waits on the action. */
      void pause();

      /** Unpause an action previously paused with `pause`. */
      void unpause();

      /** Cancel the action, removing it from its slot.
       *
       * @post The PeriodicAction object is destroyed by the slot that owns it, and the
       *     reference used to access it is no longer valid.
       */
      void cancel();
    };

  }
}

#endif	/* crisp_util_PeriodicAction_hh */

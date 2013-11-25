#include <fcntl.h>
#include <linux/input.h>
#include <crisp/input/EvDevController.hh>

namespace crisp
{
  namespace input
  {
    EvDevController::EvDevController(const char* evdev) throw ( std::system_error )
      : Controller ( ),
	m_fd ( -1 ),
	m_axis_map ( ),
	m_name ( nullptr ),
	m_location ( nullptr ),
	m_identifier ( nullptr ),
	name ( (const char*&) m_name ),
	location ( (const char*&) m_location ),
	identifier ( (const char*&) m_identifier )
    {
      if ( (m_fd = open(evdev, O_RDONLY)) < 0 )
	throw std::system_error(std::error_code(errno, std::system_category()));
      else
	{
	  char buf[BUFSIZ];
	  int sl;
	  if ( (sl = ioctl(m_fd, EVIOCGPROP(sizeof(buf)), buf)) > 0 )
	    fprintf(stderr, "properties: \"%s\"\n", buf);
	  if ( (sl = ioctl(m_fd, EVIOCGNAME(sizeof(buf)), buf)) > 0 )
	    {
	      m_name = new char[sl];
	      memcpy(m_name, buf, sl);
	    }
	  if ( (sl = ioctl(m_fd, EVIOCGPHYS(sizeof(buf)), buf)) > 0 )
	    {
	      m_location = new char[sl];
	      memcpy(m_location, buf, sl);
	    }
	  if ( (sl = ioctl(m_fd, EVIOCGUNIQ(sizeof(buf)), buf)) > 0 )
	    {
	      m_identifier = new char[sl];
	      memcpy(m_identifier, buf, sl);
	    }

	  /* Determine the available axes. */
	  size_t num_axes = 0;
	  struct {
	    size_t code;	/**< event code (Linux axis number) */
	    struct input_absinfo info;
	  } _axes[ABS_CNT];
	  memset(_axes, 0, sizeof(_axes));

	  for ( size_t i = 0; i < ABS_CNT; ++i )
	    {
	      if ( ioctl(m_fd, EVIOCGABS(i), &(_axes[num_axes].info)) >= 0 )
		{
		  if ( _axes[num_axes].info.minimum != _axes[num_axes].info.maximum )
		    _axes[num_axes++].code = i;
		}
	      else
		break;
	    }

	  /* Initialize our stored list of axes. */
	  m_axes.clear();
	  m_axes.reserve(num_axes);
	  for ( size_t i = 0; i < num_axes; ++i )
	    {
	      struct input_absinfo& info ( _axes[i].info );
	      Axis::RawConfig raw { info.value, info.minimum, info.maximum,  info.flat, info.flat };
	      m_axes.emplace_back(i, raw);
	      m_axis_map.emplace(_axes[i].code, i);
	    }

	  /* for ( size_t i ( 0 ); i < KEY_CNT; ++i )
	   *   {
	   *     struct input_keymap_entry kme;
	   *     kme.index = i;
	   *     int r ( ioctl(m_fd, EVIOCGKEYCODE_V2, &kme) );
	   *     fprintf(stderr, "%shave key %zu\n", r  >= 0 ? "" : "don't ", i);
	   *     
	   *   } */
	}

    }

    EvDevController::~EvDevController()
    {
      if ( m_name )
	delete[] m_name;
      if ( m_location )
	delete[] m_location;
      if ( m_identifier )
	delete[] m_identifier;

      m_name = nullptr;
      m_location = nullptr;
      m_identifier = nullptr;
    }


    ssize_t
    EvDevController::wait_for_event(struct input_event* ev)
    {
      return read(m_fd, ev, sizeof(struct input_event));
    }


    void
    EvDevController::run(const std::atomic<bool>& run_flag)
    {
      const char* evtypes[EV_CNT] =
	{
	  "synthetic",
	  "key",
	  "relative",
	  "absolute",
	  "misc",
	  "switch",
	  NULL,
	  NULL,
	  NULL,
	  NULL,
	  NULL,
	  NULL,
	  NULL,
	  NULL,
	  NULL,
	  NULL,
	  NULL,
	  "LED",
	  "sound",
	  NULL,
	  "repeat",
	  "force-feedback",
	  "power",
	  "force-feedback status"
	};

      struct input_event ev;
      while ( run_flag )
	{
	  if ( wait_for_event(&ev) > 0 )
	    {
	      switch ( ev.type )
		{
		case EV_ABS:
		  {			/* need these brackets so that the iterator (next line) is initialized when needed */
		    auto iter ( m_axis_map.find(ev.code) );
		    if ( iter != m_axis_map.end() )
		      axes[iter->second].post(ev.value);
		  }
		  break;

		default:
		  if ( ev.type != EV_SYN )
		    fprintf(stderr, "got %s event: code %d (0x%x), value %d (0x%x)\n", evtypes[ev.type], ev.code, ev.code, ev.value, ev.value);
		  break;
		}
	    }
	}
    }
  }
}

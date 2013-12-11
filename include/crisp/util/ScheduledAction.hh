#ifndef crisp_util_ScheduledAction_hh
#define crisp_util_ScheduledAction_hh 1

#include <boost/asio/steady_timer.hpp>
#include <functional>

namespace crisp
{
  namespace util
  {
    /* forward declaration */
    class PeriodicScheduler;

    /** Represents an action (function call) scheduled for one-time activation.
     *  ScheduledAction provides a simplified interface for canceling and/or
     *  rescheduling the action.
     */
    class ScheduledAction
    {
    public:
      typedef boost::asio::steady_timer Timer; /**< Timer type used. */
      typedef Timer::duration Duration;        /**< Timer's duration type. */
      typedef Timer::time_point TimePoint;     /**< Timer's time-point type. */

      /** Function type expected for user callbacks. */
      typedef std::function<void(ScheduledAction&)> Function;

    private:
      friend class PeriodicScheduler;
      friend class std::hash<ScheduledAction>;

      ScheduledAction(PeriodicScheduler& scheduler, Function function);

      PeriodicScheduler& m_scheduler;
      Timer* m_timer;
      Function m_function;

      void timer_expiry_handler(const boost::system::error_code& error);

    public:
      /** Move constructor. */
      ScheduledAction(ScheduledAction&& sa);
      ~ScheduledAction();

      /** Reschedule the timer to expire after the specified duration.  If
       * called while the timer is running, the previously-scheduled invocation
       * of the user callback will be canceled.
       *
       * @param duration Time after which the timer should expire.
       */
      void reset(Duration duration);

      /** Reschedule the timer to expire at the specified time.  If called while
       * the timer is running, the previously-scheduled invocation of the user
       * callback will be canceled.
       *
       * @param when Time at which the timer should expire.
       */
      void reset(TimePoint when);

      /** Cancel the action.  If called before the timer expires, this will
       *  prevent the user callback from being called.
       *
       * @post The ScheduledAction object is destroyed by the originating
       *   Scheduler object and the reference used to access it is no longer
       *   valid.
       */
      void cancel();

      /** Equality operator provided to allow use with std::unordered_set. */
      bool operator ==(const ScheduledAction& sa) const;
    };

  }
}

#endif  /* crisp_util_ScheduledAction_hh */
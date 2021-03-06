/** @file
 *
 * Contains implementation of ScheduledAction, an abstraction on top of the
 * Boost.Asio timer API that provides a simple timeout-triggered callback
 * invocation mechanism.
 *
 * This file does *not* contain the documentation for ScheduledAction; that
 * lives in the first header file included directly below this comment.
 */
#include <crisp/util/ScheduledAction.hh>
#include <crisp/util/Scheduler.hh>

namespace crisp
{
  namespace util
  {
    ScheduledAction::ScheduledAction(Scheduler& scheduler, Function function)
      : m_scheduler ( scheduler ),
        m_timer ( new Timer(scheduler.get_io_service()) ),
        m_function ( function )
    {
      assert(m_timer != nullptr);
      assert(static_cast<bool>(m_function));
    }

    ScheduledAction::ScheduledAction(ScheduledAction&& sa)
      : m_scheduler ( sa.m_scheduler ),
        m_timer ( std::move(sa.m_timer) ),
        m_function ( std::move(sa.m_function) )
    {
      sa.m_timer = nullptr;
      assert(m_timer != nullptr);
      assert(static_cast<bool>(m_function));

      TimePoint
        when_expires = m_timer->expires_at(),
        now = std::chrono::steady_clock::now();
      if ( when_expires > now )
        m_timer->async_wait(std::bind(&ScheduledAction::timer_expiry_handler, this,
                                      std::placeholders::_1));

    }

    ScheduledAction::~ScheduledAction()
    {
      if ( m_timer )
        {
          m_timer->cancel();
          delete m_timer;
          m_timer = nullptr;
        }
    }

    void
    ScheduledAction::timer_expiry_handler(const boost::system::error_code& error)
    {
      if ( m_timer && ! error )
        {
          if ( m_function )
            m_function(*this);

          /* If the timer wasn't reset or rescheduled by the user callback, ask
             the scheduler to remove this action from its list. */
          TimePoint
            when_expires = m_timer->expires_at(),
            now = std::chrono::steady_clock::now();

          if ( when_expires < now )
            cancel();
        }
    }

    void
    ScheduledAction::cancel()
    {
      m_timer->cancel();
      m_scheduler.get_io_service()
        .post(std::bind(static_cast<void(Scheduler::*)(std::weak_ptr<ScheduledAction>)>(&Scheduler::remove),
                        &m_scheduler, get_pointer()));
    }

    void
    ScheduledAction::reset(ScheduledAction::Duration duration)
    {
      m_timer->expires_from_now(duration);
      m_timer->async_wait(std::bind(&ScheduledAction::timer_expiry_handler, this,
                                    std::placeholders::_1));
    }

    void
    ScheduledAction::reset(ScheduledAction::Duration duration, ScheduledAction::Function function)
    {
      m_function = function;
      m_timer->expires_from_now(duration);
      m_timer->async_wait(std::bind(&ScheduledAction::timer_expiry_handler, this,
                                    std::placeholders::_1));
    }

    void
    ScheduledAction::reset(ScheduledAction::TimePoint when)
    {
      m_timer->expires_at(when);
      m_timer->async_wait(std::bind(&ScheduledAction::timer_expiry_handler, this,
                                    std::placeholders::_1));
    }

    void
    ScheduledAction::reset(ScheduledAction::TimePoint when, ScheduledAction::Function function)
    {
      m_function = function;
      m_timer->expires_at(when);
      m_timer->async_wait(std::bind(&ScheduledAction::timer_expiry_handler, this,
                                    std::placeholders::_1));
    }

    bool
    ScheduledAction::operator ==(const ScheduledAction& sa) const
    {
      return
        m_timer->expires_at() == sa.m_timer->expires_at() &&
        m_function.target<void>() == sa.m_function.target<void>();
    }
  }
}

#include <crisp/util/PeriodicScheduler.hh>
#include <cassert>
namespace crisp
{
  namespace util
  {

    PeriodicScheduler::PeriodicScheduler(boost::asio::io_service& io_service)
      : m_io_service ( io_service ),
        m_slots ( )
    {}

    PeriodicScheduler::~PeriodicScheduler()
    {}

    ScheduledAction&
    PeriodicScheduler::set_timer(ScheduledAction::Duration duration, ScheduledAction::Function function)
    {
      ScheduledAction action ( *this, function );
      action.reset(duration);
      std::pair<ActionSet::iterator,bool>
        action_pair ( m_actions.insert(std::move(action)) );
      assert(action_pair.second);
      return const_cast<ScheduledAction&>(*(action_pair.first));
    }

    PeriodicAction&
    PeriodicScheduler::schedule(PeriodicScheduleSlot::Duration interval,
                                PeriodicAction::Function function)
    {
      SlotMap::iterator iter ( m_slots.find(interval) );
      Slot* slot ( NULL );

      /* Look for a matching slot. */
      if ( iter != m_slots.end() )
        slot = &(iter->second);
      else
        {
          /* Add a new slot running at the specified interval  */
          std::pair<SlotMap::iterator, bool>
            slotpair = m_slots.emplace(std::make_pair(interval, Slot { *this, interval }));
          assert(slotpair.second);

          slot = &(slotpair.first->second);
        }
      
      assert(slot != NULL);

      /* Add the action to the slot. */
      PeriodicAction& out ( slot->emplace(function) );
      assert(! slot->empty() );

      return out;
    }

    void
    PeriodicScheduler::remove(const ScheduledAction& action)
    {
      m_actions.erase(action);
    }

    boost::asio::io_service&
    PeriodicScheduler::get_io_service()
    {
      return m_io_service;
    }
  }
}

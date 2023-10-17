
#include "lowlevel.hh"
#include "log.hh"
#include "relays.hh"
#include "zawor.hh"
#include "storage.hh"
#include "diag.hh"
#include "temp.hh"
#include "utils.hh"
#include "inputs.hh"
#include "states.hh"
#include "emergency.hh"


void pre_update()
{
    emergencyUpdate();
    Time::update_start();
    Diag::update();
    Zawor::powrotu.update();
    Zawor::podl1.update();
    Zawor::podl2.update();
}

void post_update()
{
    Storage::update();
}

void startup_event()
{
    INF("Controller startup");
    Storage::init();
    pre_update();
    setState(stateStartup);
    post_update();
}

void button_event(int index, bool state)
{

}

void timeout_event()
{
    pre_update();
    updateState();
    post_update();
}

void comm_event(uint8_t* data, int size)
{
    
}

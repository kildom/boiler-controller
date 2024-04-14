
#include <stdio.h>

#define async
#define await dummy_await = 
#define event if
#define enter
#define leave

void *dummy_await;
void* sleep(int time);

int expectedTemp1 = 3200;
int expectedTemp2 = 3400;
int temp1 = 2900;
int temp2 = 2900;
int hist = 100;
int podlMul = 1000;
int returnMul = 2000;
int minReturnTemp = 5500;
int criticalReturnTemp = 4800;
int returnTemp = 5700;
int returnHist = 200;

int delayTime = 9000;
int minWorkTime = 700;
int maxWorkTime = 3000;

template<typename T>
static T min(T a, T b) {
    return a < b ? a : b;
}

template<typename T>
static T max(T a, T b) {
    return a > b ? a : b;
}

template<typename T>
static T abs(T a) {
    return a > 0 ? a : -a;
}

template<typename T>
static T range(T from, T to, T value) {
    return value < from ? from : value > to ? to : value;
}


int calcWorkTime(int diff, int hist, int mul) {
    int sign = diff >> (sizeof(diff) * 8 - 1);
    diff = max(0, diff * sign - hist);
    if (diff == 0) return 0;
    return sign * range(minWorkTime, maxWorkTime, diff * mul);
}

/**
 * @param diff              [°C/100] Różnica temperatur
 * @param hist              [°C/100] Histereza +/-
 * @param proportionalDiff  [°C/100] Różnica temperatur, poniżej której sterowanie zaczyna być proporcjonalny do różnicy
 * @returns                 [%]      Procent maksymalnego sterowania, ze znakiem, może przekraczać 100%
*/
int calcProportionalSingle(int diff, int hist, int proportionalDiff) {
    int sign = diff >> (sizeof(int) * 8 - 1);
    diff = max(0, diff * sign - hist) * 100 / proportionalDiff;
    return diff * sign;
}

int proportionalDiffTemp = 100;
int proportionalDiffReturn = 200;
int tempHist = 50;

int calcProportionalWithReturn(int temp, int expectedTemp, int maxValue, int multiplier) {
    return min(propT, propR);
}

template<typename T>
T sign(T a) {
    return a >> (sizeof(T) * 8 - 1);
}

void zaw(int index, int dir);

bool returnProtect;

static const int baseTemp = 1800; // [°C/100]
static const int balancingSignal = 5; // [%]

struct VelDiscreteState {
    long lastUpdateTime;  // [absolute ms] time when the last update was made
    int activateLimit;    // 
    int value;
    int direction;
};

long time;

struct TimerContext {
    int todo;
};

VelDiscreteState velDiscreteState[2];
TimerContext velTimerContext[2];

void setTimer(TimerContext* ctx, int time);


void valDiscrete(int signal, int index) {
    VelDiscreteState* state = &velDiscreteState[index];
    // calculate time since last update
    int t = min((int)(time - state->lastUpdateTime), 500);
    state->lastUpdateTime = time;
    // calculate curves parameters
    int activateMax = 100 * delayTime;
    int activateSlope = 100 * minWorkTime / maxWorkTime;
    int activateMin = activateSlope * delayTime;
    // adjust state value with current signal
    state->value += signal * t;
    // adjust limiting curve
    state->activateLimit -= activateSlope * t;
    if (state->activateLimit < activateMin) {
        state->activateLimit = activateMin;
    }
    int valueSign = state->value >> (sizeof(state->value) * 8 - 1);
    if (state->direction != 0) {
        // adjust state value with current valve direction
        state->value = state->direction * activateMax / maxWorkTime * t;
        // if state value crossed zero, stop valve
        if (valueSign != -state->direction) {
            state->direction = 0;
            state->activateLimit = activateMax + (activateMax - activateMin);
        }
    } else if (abs(state->value) > state->activateLimit) {
        // if state value reached limiting curve, start valve
        state->direction = -valueSign;
    }
    if (state->direction != 0) {
        setTimer(&velTimerContext[index], abs(state->value) * maxWorkTime / activateMax + 50);
    }
    zaw(index, state->direction);
}

void velUpdate()
{
    // Sterowanie proporcjonalne dla każdego zaworu
    int propT1 = calcProportionalSingle(expectedTemp1 - temp1, tempHist, proportionalDiffTemp);
    int propT2 = calcProportionalSingle(expectedTemp2 - temp2, tempHist, proportionalDiffTemp);
    // Uwzględnij sterowanie proporcjonalne dla ochrony powrotu
    if (returnProtect) {
        int propR = calcProportionalSingle(returnTemp - minReturnTemp, returnHist, proportionalDiffReturn);
        propT1 = min(propT1, propR);
        propT2 = min(propT2, propR);
    }
    // Dodaj sterowanie balansujące oba zawory
    int minBalancingTemp = baseTemp + 2 * tempHist;
    if (expectedTemp1 > minBalancingTemp && expectedTemp2 > minBalancingTemp && temp1 > minBalancingTemp && temp2 > minBalancingTemp)
    {
        // Skaluj temperatury tak, żeby były porównywalne
        int mid = (expectedTemp1 + expectedTemp2) / 2 - baseTemp;
        int scaled1 = (temp1 - baseTemp) * mid / (expectedTemp1 - baseTemp);
        int scaled2 = (temp2 - baseTemp) * mid / (expectedTemp2 - baseTemp);
        // Przytnij sterowanie do maksymanych wortości przed dodawanie syg. balansującego
        propT1 = range(-100, 100, propT1);
        propT2 = range(-100, 100, propT2);
        // Jeżeli różnica powyżej histerezy, dodaj mały stały synał do jednego, a odejmij go od drugiego
        int diff = scaled1 - scaled2;
        int balance = (diff < -tempHist) ? balancingSignal : (diff > tempHist) ? -balancingSignal : 0;
        propT1 += balance;
        propT2 -= balance;
        // Jeżeli wyszliśmy poza zakres, przekaż część sygnału do drugiego
        if (abs(propT1) > 100)
        {
            int overflow = (abs(propT1) - 100) * sign(propT1);
            propT1 -= overflow;
            propT2 += overflow;
        }
        if (abs(propT2) > 100)
        {
            int overflow = (abs(propT2) - 100) * sign(propT2);
            propT2 -= overflow;
            propT1 += overflow;
        }
    }
    // Przytnij sterowanie do maksymanych wortości
    propT1 = range(-100, 100, propT1);
    propT2 = range(-100, 100, propT2);
    // Dyskretyzuj sygnał
    valDiscrete(propT1, 0);
    valDiscrete(propT2, 1);
}

enum StateStage {
    STAGE_ENTER = 0,
    STAGE_EXIT = 1,
    STAGE_UPDATE = 3,
};

static const int START_TIME = 240 * 1000;

void stateStarting(StateStage stage)
{
    static long startTime = time;
    if (stage == STAGE_ENTER) {
        startTime = time;
        zaw_idle(0);
        zaw_idle(1);
        zaw_idle(2);
        pomp(0, 0);
        pomp(1, 0);
        pomp(2, 1);
        fuel(0);
        pellet(0);
        elektr(0);
    } else if (stage == STAGE_EXIT) {
        zaw(0, 0);
        zaw(1, 0);
        zaw(2, 0);
        pomp(2, 0);
        return;
    }
    /*if (checkOverheat(true)) { TODO: checking overheat and other alarms should be done always outside state functions.
        return;
    }*/
    if (time - startTime > max(START_TIME, openTime + openTime / 2)) {
        setState(inputElek || emergencyElek ? stateElekrRunning : statePelletRunning);
        return;
    }
}

static long long pelletStartTime;
static int MAX_PELLET_HEAT_UP_TIME = 45 * 60 * 1000;

void statePelletRunning(StateStage stage)
{
    static void* resumePoint = NULL;

    if (stage == STAGE_ENTER) {
        fuel(0);
        // ...
        resumePoint = &&entry;
    } else if (stage == STAGE_EXIT) {
        //
        return;
    }

    if (inputElek || emergencyElek) {
        setState(statePelletStopping);
        return;
    }

    goto *resumePoint;

    // sub-state: Wait for ready and request to heat
entry:
    if (!zaw_ready(0) || !zaw_ready(1) || !zaw_ready(2)) return;
    if (!inputRoom && !(cwuEnabled && cwuTemp < cwuRequired - cwuHist)) return;

    // Starting
    pelletStartTime = time;
    fuel(true);
    pellet(true);
    pomp(2, true);

    // sub-state: Wait for heat up
start:
    resumePoint = &&start;
    if (time - pelletStartTime > MAX_PELLET_HEAT_UP_TIME) {
        startEmergencyElek();
        setState(statePelletStopping);
        return;
    }
    if (T_return < minReturnTemp) return;


}

static const int ELEKTR_POMP_RUNNING = 20 * 1000;

static long long pompRunningTime = 0;

void stateElekrRunning(StateStage stage) // TODO: tryb elek. z CWU
{
    if (stage == STAGE_ENTER) {
        fuel(0);
        zaw_max(0);
        zaw_max(1);
        zaw_max(2);
        pellet(0);
        elektr(0);
    } else if (stage == STAGE_EXIT) {
        elektr(false);
        return;
    }

    if (zaw_ready(0) && zaw_ready(1) && zaw_ready(2) && inputRoom) {
        elektr(true);
        pomp(0, true);
        pomp(1, true);
        pompRunningTime = time + ELEKTR_POMP_RUNNING;
    } else {
        elektr(false);
    }

    if (time > pompRunningTime) {
        pomp(0, false);
        pomp(1, false);
    }

    if (!inputElek && !emergencyElek) { // TODO: outside general logic: if (inputElek && emergencyElek) stopEmergencyElek();
        setState(stateElekrStopping);
    }
}

void stateElekrStopping(StateStage stage)
{
    elektr(false);

    if (time > pompRunningTime) {
        setState(stateStarting);
    }
}

async void* vel() {

    normalWork:
    {
        event (returnTemp < minReturnTemp - returnHist) goto returnRecovery;

        while (true) {
            int $workTime;
            await sleep(delayTime);
            $workTime = min(
                calcWorkTime(expectedTemp - temp, hist, podlMul),
                calcWorkTime(, returnHist, returnMul)
            );
            if ($workTime > 0) {
                zaw(+1);
                await sleep($workTime);
            } else if ($workTime < 0) {
                zaw(-1);
                await sleep(-$workTime);
            }
            zaw(0);
        }

        leave
        {
            zaw(0);
        }
    }

    returnRecovery:
    {
        event (returnTemp > minReturnTemp - returnHist) goto normalWork;

        while (true) {
            zaw(-1);
            await sleep(2 * maxWorkTime);
            zaw(0);
            await sleep(delayTime);
        }

        leave
        {
            zaw(0);
        }
    }
}


struct N {
    const char* name;
    N(const char* name):name(name) {
        printf("N %s\n", name);
    }
    ~N() {
        printf("~ N %s\n", name);
    }
};

void func() {

    void* const *jump_chain;
    int jump_chain_index;

    //goto jump_target;
    static void* const jc9878[] = { &&scope1, &&jump_target, NULL };
    jump_chain = jc9878;
    jump_chain_index = 0;
    goto scope0;

    scope0:
    N a("0");
    scope0_jumps:
    if (jump_chain[jump_chain_index]) goto *jump_chain[jump_chain_index++];
    scope0_directly:

    {
        scope1:
        N a("a");
        scope1_jumps:
        if (jump_chain[jump_chain_index]) goto *jump_chain[jump_chain_index++];
        scope1_directly:
        jump_target:
        printf("Target 1\n");
        // goto jump_target2
        static void* const jc9824[] = { &&scope2, &&jump_target2, NULL };
        jump_chain = jc9824;
        jump_chain_index = 0;
        goto scope0_jumps;
    }

    printf("Target none\n");

    {
        scope2:
        N a("b");
        scope2_jumps:
        if (jump_chain[jump_chain_index]) goto *jump_chain[jump_chain_index++];
        scope2_directly:
        jump_target2:
        printf("Target 2\n");
    }

}


int main() {
    func();
    return 0;
}

typedef void (*fp_t)();


template<class State>
class StateMachine
/*
* State machine class.
* Maximum amount of methods mapped: 128
*
* This object allows for continuous
* polymorphic access to state methods in
* a call stack. It aids to automate the
* transition between different function
* calls after a set of pre-determined set
* of states.
*
* The mainState and mainMethod correlate
* to the "idle" state of the device, and
* will automatically be the default yield
* whenever the previous state has exhausted
* and the method getCurrentState is called
* once again. It will remain the result
* until methods with access mutate the state
* through passing an enum instance for the
* desired state. If however the transitional
* state mismatches with the predefined
* transitional state and a method tries to
* switch to an illegal state, this is prohibited
* and automatically reverts to the main state
* and ignores the attempted transition.
*
* For custom transitions to be allowed, e.g.:
*
	* blinkLed -> runMotor
*
* .. instead of the default, which would be:
*
	* blinkLed -> mainState
*
* .. the StateMachine instance must be provided
* with this breadcrum trail upon defining the
* method in the addMethod call.
*
* It is as easy as adding the optional
* "transition" parameter in the function call,
* defining an enum to the desired transitional
* state. For this to be supported, the transitional
* state must be implemented before the state which
* uses it as a transition.
*/
{
private:

	int stateCount;
	State currentState;
	State mainState;
	fp_t mainMethod;	
	fp_t methods[128];
	State chainedStates[128];
	State states[128];
	State nextState;
	State staticState;


	const fp_t getMethodForState(State state)
		/*
		* Iterate over the states and return the method
		* on the same index position as the given state
		* in the linked arrays.
		*/
	{
		for (int i = 0; i < this->stateCount; i++)
		{
			if (states[i] == state)
			{
				return this->methods[i];
			}
		}return this->mainMethod;
	}


	const State getChainedStateForState(State state)
		/*
		* Get the next state in line, as per defined
		* by the given state the machine is currently
		* in at the time of calling. The default is
		* the mainState state as a failsafe fallback,
		* if loop exhausts without finding a matching
		* transition for the current state.
		*/
	{
		for (int i = 0; i < this->stateCount; i++)
		{
			if (this->states[i] == state)
			{
				return this->chainedStates[i];
			}
		}
		return this->mainState;
	}

public:


	StateMachine(State state, fp_t mainMethod)
	{
		this->mainMethod = mainMethod;
		this->mainState = state;
		this->staticState = state;
	}

	const State getMainState()
		/*
		* Returns the main state of the State Machine.
		*/
	{
		return this->mainState;
	}


	const fp_t next()
		/*
		* Allows for a continuous polling method to be called
		* which returns the method for the state the machine
		* is in at the time of call. The previous state is cached
		* as to reduce lookup time and CPU cycles spent on searching,
		* if the state from the previous call is left unchanged.
		*
		* Protects the application from an infinite loop where
		* the state is left unchanged by misbehaving state methods
		* which do not call the release() method. This validation
		* is implemented by not allowing the same state to occur twice
		* or more in sequence.
		*/
	{

		if (this->currentState == this->nextState)
		{
			this->nextState = this->mainState;
		}

		this->currentState = this->nextState;
		return this->getMethodForState(this->nextState);
	}


	const State getCurrentState()
		/*
		* Returns the state the State Machine is in
		* at the time of calling.
		*/
	{
		return this->currentState;
	}


	void release()
		/*
		* Mutates the instance's currentState to whichever
		* state is defined as the transitional state for
		* the state the machine is in at the given time
		* of call.
		*/
	{
		this->nextState = this->getChainedStateForState(this->currentState);
	}


	void transitionTo(State state)
		/*
		* Allows methods to mutate the state of the machine
		* to transition to another method in the call stack.
		* It is however only allowed if the current state is
		* the main state, that is, it cannot interrupt an
		* ongoing transition between one secondary state and
		* another.
		*/
	{
		if (this->currentState == this->mainState)
		{
			this->nextState = state;
		}
	}


	void addState(State state, fp_t func)
		/*
		* Add states to the state machine, mimics a key-value pair.
		*
		* parameter func (function pointer fp_t):
		*	Denotes which method/function to add as a state
		*
		* parameter state
		*	Denotes which state identifier to bind to the
		*   function / method defined as 'func' parameter.
		*   This can be seen as the 'key' in a key/value
		*   structure. This can be anything. A custom class,
		*   instance of custom class, int, string, etcetera.
		*   The recommended way for scaleable code is to write
		*   an enum class and use enum instance, named after the
		*   function.
		*/
	{
		if (this->stateCount < sizeof(this->methods) / sizeof(this->methods[0]))
		{
			this->methods[this->stateCount] = func;
			this->states[this->stateCount] = state;
			this->chainedStates[this->stateCount] = this->mainState;
			this->stateCount++;
		}
	}
	

	void setChainedState(State primary, State secondary)
		/*
		* Define a state which should run after a state.
		*
		* This is useful when the desired behavior is
		* a scenario where the user would like the
		* state to not return to the main state when
		* a given state is exhausted, but that another
		* state is called.
		*
		* RESTRICTIONS / IGNORED CASES:
		*
		* Both the primary and secondary state must be
		* added using the addState() method prior to
		* calling this method.
		*
		* One state cannot chain itself. If primary and
		* secondary are identical, the chain is ignored.
		*/
	{
		if (primary == secondary)
			return;

		for (int i = 0; i < sizeof(this->states) / sizeof(this->states[0]); i++)
		{
			if (this->states[i] == primary)
			{
				this->chainedStates[i] = secondary;
			}
		}
	}

	void setStaticState(State state)
	/*
	* Set a state that the StateMachine will remember.
	* The static state can then be retrieved inside
	* the main state, which can then call `transitionTo()`
	* on this state. 
	* This method is useful if a button push is supposed to
	* keep the device in that state until something else triggers
	* an abort. The StateMachine will still return to the 
	* main state function when the static state call is 
	* exhausted. The difference is that this state can be
	* polled by the caller to serve as a temporary notepad
	* of which state the device should remain in.
	*
	* This method will NOT modify the behavior of `next` 
	* it will simply keep returning the next state in line.
	*/
	{
		if (this->currentState == this->mainState)
		{
			this->staticState = state;
		}
	}

	const State getStaticState()
	/*
	* Returns the configured static state set inside
	* the main state function, if configured.
	* The State Machine will repeatedly return this
	* state unless it is mutated by the main state.
	*/
	{
		return this->staticState;
	}
};
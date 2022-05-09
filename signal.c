// Signal - Source
// Programming pattern to provide easy calls on functions
// Gavin Zimmerman

#include "signal.h"


// SOURCE CODE

void SignalSimpleHandler(void (*func)()) {
    func();
}

// SignalInit - Initializes a signal to its default connections (none)
    // @param s Signal to initialize
void SignalInit(SignalInstance* s, void (*handler)(void*, va_list) ) {
    s->Connections = 0;
    s->_handler = handler;
    for (int i=0; i<MAX_SIGNAL_CONNECTIONS; i++) {
        s->_functions[i] = NULL;
    }
}

// SignalFire - Prompts invokation on functions connected to signal
    // @param s - Signal to fire functions connect on
    // @param ... - Arguments to pass in as a va_list
void SignalFire(SignalInstance *s, ...) {
    int i=0;
    va_list args;
    
    while (s->_functions[i] != NULL) {
        va_start(args, s);
        if (s->_handler==NULL)
            SignalSimpleHandler(s->_functions[i]);
        else
            ((void(*)())(s->_handler))(s->_functions[i], args);
        va_end(args);
        i++;
    }
    
}

// SignalConnect - Connects a function to a signal
    // @param s - Signal to bind function with
    // @param func - Function to bind onto signal
int SignalConnect(SignalInstance *s, void* func) {
    if (s->Connections+1==MAX_SIGNAL_CONNECTIONS)
        return -1;
    
    s->_functions[s->Connections] = func;
    s->Connections += 1;
    return 0;
}

// SignalDisconnect - Disconnects a function from the signal
    // @param s - Signal to disconnect function from
    // @param func - Function to disconnect
int SignalDisconnect(SignalInstance *s, void* func) {
    for (int i=0; i<s->Connections; i++) {
        if (s->_functions[i]!=func) 
            continue;
        
        for (; i<s->Connections-1; i++) 
            s->_functions[i]=s->_functions[i+1];
    
        s->Connections--;
        s->_functions[s->Connections]=NULL;
        return 0;
    }
    return -1;
}

// Signal Destroy - Erases references to any connected functions
    // @param s - Signal to clean contents of
void SignalDestroy(SignalInstance *s) {
    for (int i=0; i<MAX_SIGNAL_CONNECTIONS; i++)
        s->_functions[i]=NULL;
    s->Connections=0;
}
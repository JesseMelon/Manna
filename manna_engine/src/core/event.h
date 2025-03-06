#pragma once

#include "defines.h"

//TODO: deferred events. "It's rare- if ever- that you need the message to be sent right now in the frame when you call fire event(). 
//Especially inside hot loops it makes a lot of sense to keep your cache hot- and event invocations are notorious for just going off 
//and doing something else in the middle of something important. If you defer your call (basically adding your eventcontext to another 
//of your darrays) you can fire them all at once when you're ready. What I mean by that is making your event handler function signature 
//include an array of contexts- this batching makes threading on the handler side much easier, as well as opening up other optimizations 
//like SIMD that are much trickier with the "one at a time" vanilla events." - youtube

typedef union event_data {
    u64 i64[2];
    u64 u64[2];
    f64 f64[2];

    i32 i32[4];
    u32 u32[4];
    f32 f32[4];

    i16 i16[8];
    u16 u16[8];

    i8 i8[16];
    u8 u8[16];

    char c[16];
} event_data;


/**
 * @brief Function header for event callbacks
 *
 * @param[in] event_id Unique event identifier
 * @param[in] sender Pointer to what triggered event
 * @param[in] listener_instance Pointer to the instance to handle the event
 * @param[in] data Arbitrary event data, 128 bits.
 * @return TRUE if event was processed, FALSE otherwise
 */
typedef b8 (*event_handler)(u16 event_id, void* sender, void* listener_instance, event_data data);

b8 init_events();
void shutdown_events();

/**
 * @brief Register to listen for an event. When the event occurs, on_event is invoked.
 *
 * @param[in] event_id The id of the event to listen to.
 * @param[in] listener Pointer to the listening instance. Can be NULL.
 * @param[in] on_event The callback function pointer to be invoked by the event.
 * @return TRUE if successfully registered. FALSE if already listening, or failure.
 */
MANNA_API b8 listen_to_event(u16 event_id, void* listener, event_handler on_event);

/**
 * @brief Unregister from event.
 *
 * @param[in] event_id The id of the event to ignore
 * @param[in] listener The instance to un-register. Can be NULL.
 * @param[in] on_event The callback function pointer to be unregistered
 * @return TRUE if successfully unregistered; FALSE otherwise.
 */
MANNA_API b8 ignore_event(u16 event_id, void* listener, event_handler on_event);

/**
 * @brief Invokes all listeners of given event. The last event handler should return TRUE indicating the event is complete.
 *
 * @param[in] event_id The id of the even to be triggered
 * @param[in] sender A pointer to the event sender. Can be NULL.
 * @param[in] event_data A 128 bit union data type to contain arbitrary event data
 * @return TRUE if handled; FALSE otherwise.
 */
MANNA_API b8 trigger_event(u16 event_id, void* sender, event_data event_data);


typedef enum internal_event_id {
    EVENT_APPLICATION_QUIT = 0x01,
    
    /* event_data usage:
    * u16 key = event_data.u16[0];
    */
    EVENT_KEY_PRESSED = 0x02,

    /* event_data usage:
    * u16 key = event_data.u16[0];
    */
    EVENT_KEY_RELEASED = 0x03,

    /* event_data usage:
    * u16 button = event_data.u16[0];
    */
    EVENT_MOUSE_BUTTON_PRESSED = 0x04,

    /* event_data usage:
    * u16 button = event_data.u16[0];
    */
    EVENT_MOUSE_BUTTON_RELEASED = 0x05,

    /* event_data usage:
    * u16 x = event_data.u16[0];
    * u16 y = event_data.u16[1];
    */
    EVENT_MOUSE_MOVED = 0x06,

    /* event_data usage:
    * u8 delta = data.u8[0];
    */
    EVENT_MOUSE_WHEEL = 0x07,

    /* event_data usage:
    * u16 width = data.u16[0];
    * u16 height = data.u16[1];
    */
    EVENT_WINDOW_RESIZED = 0x08, 

    MAX_EVENT_ID = 0xFF
} internal_event_id;

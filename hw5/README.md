# Homework 5 - CSE 320 - Spring 2018
#### Professor Eugene Stark

### **Due Date: Friday 05/04/2018 @ 11:59pm**

## Updates

If updates are made to this document or the base code, they will be
noted in this section.  Remember that base code changes require you to
fetch and merge as described in `hw0`.  All updates to this document
are prefaced with *UPDATE*.

## Introduction

The goal of this assignment is to become familiar with low-level POSIX
threads, multi-threading safety, concurrency guarantees, and
networking.  The overall objective is to implement a simple multi-threaded
chat server.  As this is more difficult than it might sound, to ease
you along we have provided you with a design for the server, as well as
binary object files for almost all the modules.  This means that you
can build a functioning server without initially facing too much
complexity.  In each step of the assignment, you will replace one of our
binary modules with one built from your own source code.  If you succeed
in replacing all of our modules, you will have completed your own
version of the server.

### Takeaways

After completing this homework, you should:

* Understand thread execution, locks, and semaphores
* Have an advanced understanding of POSIX threads
* Insight into the design of concurrent data structures
* Have a basic understanding of socket programming
* Have enhanced your C programming abilities

## Hints and Tips

* We strongly recommend you check the return codes of all system
  calls. This will help you catch errors.

* **BEAT UP YOUR OWN CODE!** Throw lots of concurrent calls at your
  data structure libraries to ensure safety.

* Your code should **NEVER** crash. We will be deducting points for
  each time your program crashes during grading. Make sure your code
  handles invalid usage gracefully.

* You should make use of the macros in `debug.h`. You would never
  expect a library to print arbitrary statements as it could interfere
  with the program using the library. **FOLLOW THIS CONVENTION!**
  `make debug` is your friend.

> :scream: **DO NOT** modify any of the header files provided to you in the base code.
> These have to remain unmodified so that the modules can interoperate correctly.
> We will replace these header files with the original versions during grading.
> You are of course welcome to create your own header files that contain anything
> you wish.

> :nerd: When writing your program, try to comment as much as possible
> and stay consistent with your formatting.

## Helpful Resources

### Textbook Readings

You should make sure that you understand the material covered in
chapters **11.4** and **12** of **Computer Systems: A Programmer's
Perspective 3rd Edition** before starting this assignment.  These
chapters cover networking and concurrency in great detail and will be
an invaluable resource for this assignment.

### pthread Man Pages

The pthread man pages can be easily accessed through your terminal.
However, [this opengroup.org site](http://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread.h.html)
provides a list of all the available functions.  The same list is also
available for [semaphores](http://pubs.opengroup.org/onlinepubs/7908799/xsh/semaphore.h.html).

## Getting Started

Fetch and merge the base code for `hw5` as described in `hw0`. You can
find it at this link: https://gitlab02.cs.stonybrook.edu/cse320/hw5.
Remember to use the `--stategy-option theirs` flag for the `git merge`
command to avoid merge conflicts in the Gitlab CI file.

## The Bavarde Server and Protocol: Overview

The "Bavarde" server implements a simple chat service that allows
clients to send messages to each other.  A client wishing to make use
of the system first makes a network connection to the server.  Once
connected to the server, a client can make the following **requests**
to the server:

- Log in to the system, specifying a user-friendly **handle** to identify
  the client.

- Obtain a list of handles of all clients currently logged in to the server.

- Send a message to another client, identified by its handle.  The message
  is placed in that client's mailbox, to be delivered as soon as possible.

- Log out from the system.

In response to these requests, the server will send a **notice**,
consisting of either a positive acknowledgement (`ACK`) or a negative
acknowledgement (`NACK`).  A positive acknowledgement indicates that
the server has carried out the request; a negative acknowledgement
indicates that some error occurred.

Besides `ACK` and `NACK`, the server will send the following additional
types of notices, which are issued asynchronously and independently
of any requests that the client has made:

- Delivery of a message sent by another client to this client.

- Notice that a previously sent message has been delivered to its recipient.

- Notice that a previously sent message has bounced, because the recipient
  disconnected from the server before the message could be delivered.

A client may disconnect from the system at any time.  If the client
was logged in, then it will be logged out and its handle unregistered
from the system.  Any pending messages for that client are then
discarded, and bounce notices are sent to the senders of those
messages.

The Bavarde server architecture is that of a multi-threaded network
server.  When the server is started, a **master** thread sets up a
socket on which to listen for connections from clients.  When a
connection is accepted, a **client service thread** is started to
handle requests sent by the client over that connection.  The client
service thread executes a service loop in which it repeatedly receives
a **request packet** sent by the client, performs the request, and
sends either an `ACK` or `NACK` packet in response.

One of the requests that the client can make is a `LOGIN` request,
which contains a handle by which the client wishes to be identified.
In the case of a successful login, an `ACK` packet is sent to the
client.  If the requested handle is already in use by another client,
then the server responds with a `NACK` packet.  If the requested
handle is not already in use, a **mailbox** is created for the client
and a mapping from the handle to the mailbox is added to the
**directory** of logged-in clients.  In addition, a **mailbox service
thread** is started to handle messages and notices posted to the
mailbox. The mailbox service thread executes a service loop in which
it repeatedly receives a message or notice from the mailbox and sends
it over the network connection to the client.  The service loop
continues to execute until the mailbox is shut down as a result of the
disconnection of the client.  At that point, the mailbox service
thread terminates.

> :nerd: One of the basic tenets of network programming is that a
> network connection can be broken at any time and the parties using
> such a connection must be able to handle this situation.  In the
> present context, the client's connection to the Bavarde server may
> be broken at any time, either as a result of explicit action by the
> client or for other reasons.  When disconnection of the client is
> noticed by the client service thread, the client's handle is
> unregistered from the directory, the client's mailbox is shut down,
> and the client service thread terminates.  Once the mapping from the
> handle to the mailbox has been removed from the directory, the
> mailbox can no longer be looked up; however at the time of removal
> it might be the case that references to the mailbox are still being
> held by threads that are in the process of sending a message to that
> mailbox.  The mailbox cannot be freed until no such references
> exist.  This issue creates a design complication that will be
> explained further below.

A second kind of request that the client can make is a `USERS`
request, which asks the server for a list of the handles of currently
connected clients.  Upon receipt of such a request, the server queries
the directory to obtain a list of the currently registered handles.
It then sends the client an `ACK` packet that contains this list.

The third kind of request that the client can make is a `SEND`
request, which asks the server to send a message to another client,
identified by its handle.  Upon receipt of such a request, the server
checks that the sender is currently logged in, that the specified
recipient handle is registered with the directory, and it obtains the
sender and recipient mailboxes from the directory.  A message is then
constructed that contains a reference to the sender's mailbox in a
"from" field and the content to be sent in a "body" field.  (The
reason for storing a reference to the sender's mailbox in the message,
rather than just the sender's handle, is explained further below.)
The newly constructed message is added to the recipient's mailbox,
where it will ultimately be discovered by the recipient's mailbox
service thread and delivered to the recipient.  If the above procedure
succeeds, an `ACK` is sent to the sending client; otherwise `NACK` is
sent.

Finally, a client can make a `LOGOUT` request, which asks the server
to log out the client.  Upon receipt of such a request, if the client
was logged in, then the server performs the same actions as if the client
had disconnected.  If the client was not logged in, `shutdown()` is
called on the client socket file descriptor.  No acknowledgement is sent
in either case.

### The Base Code

Here is the structure of the base code:

```
hw5
├── client
│   └── client
├── include
│   ├── debug.h
│   ├── directory.h
│   ├── mailbox.h
│   ├── protocol.h
│   ├── server.h
│   └── thread_counter.h
├── lib
│   └── bavarde.a
├── Makefile
├── src
│   └── main.c
└── tests
    └── bavarde_tests.c
```

The base code consists of header files that define module interfaces,
a library `bavarde.a` containing binary object code for our
implementations of the modules, and a source code file `main.c` that
contains containing a stub for function `main()`.  The `Makefile` is
designed to compile any existing source code files and then link them
against the provided library.  The result is that any modules for
which you provide source code will be included in the final
executable, but modules for which no source code is provided will be
pulled in from the library.

The `client` directory contains an executable for a simple test
client.  When you run this program it will print a help message that
summarizes the commands.  The client program understands command-line
options `-h <hostname>`, `-p <port>`, and `-q`.  The `-p` option is
required; the others are options.  The `-q` flag tells the client to
run without prompting; this is useful if you want to run the client
with the standard input redirected from a file.

## Task I: Server Initialization

When the base code is compiled and run, it will print out a message
saying that the server will not function until `main()` is
implemented.  This is your first task.  The `main()` function will
need to do the following things:

- Obtain the port number to be used by the server from the command-line
  arguments.  The port number is to be supplied by the required option
  `-p <port>`.
  
- Install a `SIGHUP` handler so that clean termination of the server can
  be achieved by sending it a `SIGHUP`.  Note that you need to use
  `sigaction()` rather than `signal()`, as the behavior of the latter is
  not well-defined in a multithreaded context.

- Set up the server socket and enter a loop to accept connections
  on this socket.  For each connection, a thread should be started to
  run function `bvd_client_service()`.

These things should be relatively straightforward to accomplish, given the
information presented in class and in the textbook.  If you do them properly,
the server should function and accept connections on the specified port,
and you should be able to connect to the server using the test client.
Note that if you build the server using `make debug`, then the binaries
we have supplied will produce a fairly extensive debugging trace of what
they are doing.  This, together with the specifications in this document
and in the header files, should help you to understand the behavior of the
various modules.

## Task II: Send and Receive Functions

The header file `include/protocol.h` defines the format of the packets
used in the Bavarde network protocol.  The concept of a protocol is an
important one to understand.  A protocol creates a standard for
communication so that any program implementing a protocol will be able
to connect and operate with any other program implementing the same
protocol.  Any client should work with any server if they both
implement the same protocol correctly.  In the Bavarde protocol,
clients and servers exchange **packets** with each other.  Each packet
has two parts: a required **header** that describes the packet, and an
optional **payload** that can carry arbitrary data.  Packet headers
always have the same size and format, which is given by the
`bvd_packet_header` structure; however the payload can be of arbitrary
size.  One of the fields in the header tells how long the payload is.

- The function `proto_send_packet` is used to send a packet over a
network connection.  The `fd` argument is the file descriptor of a
socket over which the packet is to be sent.  The `hdr` argument is a
pointer to the header of the packet.  The `payload` argument is a
pointer to the payload, if there is one, otherwise it is `NULL`.  The
`proto_send_packet` assumes that multi-byte fields in the packet
passed to it are in **host byte order**, which is the normal way that
values are stored in variables.  However, as byte ordering
(i.e. "endianness") differs between computers, before sending the
packet it is necessary to convert any multi-byte fields to **network
byte order**.  This can be done using, e.g., the `htonl()` and related
functions described in the Linux man pages.  Once the header has been
converted to network byte order, the `write()` system call is used to
write the header to the "wire" (i.e. the network connection).  If the
length field of the header specifies a nonzero payload length, then an
additional `write()` call is used to write the payload data to the
wire.

- The function `proto_recv_packet()` reverses the procedure in order to
receive a packet.  It first uses the `read()` system call to read a
packet header from the wire.  After converting multi-byte fields in
the header from network byte order to host byte order (see the man
page for `ntohl()`), if the length field of the header is nonzero then
an additional `read()` is used to read the payload from the wire.  The
header and payload are stored using pointers supplied by the caller.

**NOTE:** Remember that it is always possible for `read()` and `write()`
to read or write fewer bytes than requested.  You must check for and
handle these "short count" situations.

Implement these functions in a file `protocol.c`.  If you do it
correctly, the server should function as before.

## Task III: Thread Counter

You probably noticed the initialization of the `thread_counter`
variable in `main()` and the use of the `tcnt_wait_for_zero()`
function in `terminate()`.  The thread counter provides a way of
keeping track of the number of server threads that currently exist,
and to allow a "master" thread to wait for the termination of all
server threads before finally terminating execution.

The functions provided by a thread counter are specified in the
`thread_counter.h` header file.  Provide implementations for these
functions in a file `src/thread_counter.c`.  Note that these functions
need to be thread-safe, so synchronization will be required.  Use a
mutex to protect access to the thread counter data.  Use a semaphore
to perform the required blocking in the `tcnt_wait_for_zero()`
function.

Implementing the thread counter should be a fairly easy warm-up
exercise in concurrent programming.  If you do it correctly, the
Bavarde server should still shut down cleanly in response to SIGHUP
using your version.

**Note:** You should test your thread counter separately from the
server.  Create test threads that rapidly call `tcnt_incr()` and
`tcnt_decr()` methods concurrently and then check that a call to the
`tcnt_wait_for_zero()` function blocks until the count reaches zero,
and then returns.

## Task IV: Directory

The next task is to implement the directory.  This will be slightly
harder than the thread counter, but it shouldn't be too bad.
The functions to be implemented are specified in the file `include/directory.h`,
and the implementation should be in a file `src/directory.c`.
You are free to choose the data structure used to store the
directory entries, as well as the specific content of those entries.
Note once again that these functions will be called concurrently,
so they have to be implemented in a thead-safe way.

- Function `dir_register()` takes as arguments the handle to be
registered and the file descriptor of the socket used to communicate
with the client.  It needs to create a mailbox (use `mb_init()` for
this), and both the mailbox and the file descriptor need to be stored
in the directory.  The mailbox is needed, obviously, in order to send
a message the client.  The file descriptors are stored in the
directory as well as part of the scheme for shutting down the server
cleanly.  The `dir_shutdown()` function goes through each of the
entries in the directory and calls the `shutdown()` function on its
socket file descriptor (see the man page for `shutdown(2)`).  The
shutdown of the sockets will trigger the termination of the server
threads and ultimately lead to a clean server shutdown.

- Functions `dir_register()` and `dir_lookup()` must call
`mb_ref()` to increase the reference count on a mailbox before
returning a pointer to it.  This is because returning the pointer to
the caller increases the number of outstanding pointers to the
mailbox, so the reference count has to be increased to maintain the
required correspondence between the reference count and the number of
existing pointers.  If you don't do this correctly, it will affect
whether a mailbox gets finalized properly when a client disconnects.
More information on the reference-counting scheme used by mailboxes is
given in the "Mailbox" section below.

- Function `dir_unregister()` similarly must call `mb_unref()`
on the mailbox being removed from the directory.  This is because its
removal leaves one fewer pointers to the mailbox.

- Function `dir_all_handles()` is supposed to return a list
of all handles that were registered at the time it was called.
However, as soon this function returns, the list could become
outdated as a result of clients registering or unregistering.
So the caller of this function can rely only on the returned list
being a snapshot of the state of the directory at some previous time.

## Task V: Mailboxes

Next, implement mailboxes.  The mailbox functions are specified in
`include/mailbox.h` and they should be implemented in a file
`src/mailbox.c`.  Once again, keep in mind that all these functions
have be thread-safe.

The core mailbox functions are `mb_add_message()`, `mb_add_notice()`,
and `mb_next_entry()`, which enqueue and remove mailbox entries.  The
behavior of these functions follows the producer/consumer paradigm
(see section **12.5** in **Computer Systems: A Programmer's
Perspective 3rd Edition**) in the sense that callers of
`mb_add_message()` and `mb_add_notice()` **produce** entries to be
added to a mailbox and callers of `mb_next_entry()` **consume**
entries from the mailbox.  If a consumer tries to consume an entry
from an empty mailbox, then it should block until an entry becomes
available.  Similarly, if there were a maximum capacity specified for
a mailbox, then a producer trying to insert an entry into the mailbox
should block until there is space.  For this assignment, we have not
set a maximum capacity for a mailbox (though it would be essential do
so before deploying this server in a real networking environment) so
you don't have to worry about producers blocking.

The design of the mailbox module addresses some issues that make it
somewhat more complex than, say, the directory module.

The first issue has to do with reference counts.  Normally, mailboxes
reside in entries in the directory; however, when one client requests
that a message be sent to another the recipient's mailbox has to be
retrieved using its handle.  The `dir_lookup()` function returns a
pointer to the recipient's mailbox to its caller, which will be the
client service thread that is serving the sender's connection.
While the message is actually being sent, the mailbox pointer will
exist outside of the directory.  Now consider what happens if the
recipient logs out or is disconnected while this is going on.
This situation will be noticed by the client service thread for the
recipient's network connection.
We want the recipient's mailbox to be freed, but it is not safe to
do so as long as a pointer to the mailbox exists through which the
mailbox might be accessed from another thread.  We need some way to
delay the freeing of a mailbox until no such pointers exist.

To address this issue, we use a **reference counting** scheme.
The basic idea of reference counting is for an object to maintain
a field that counts the total number of pointers to it that exist.
Each time a pointer is created (or copied) the reference count is
increased.  Each time a pointer is discarded, the reference count
is decreased.  When the reference count reaches zero, there are
no outstanding pointers to the object and it can be freed.
To apply such a scheme, the object to which reference counting
is to be applied has to provide two functions: one for increasing
the reference count and another for decreasing it.  The function
for decreasing the reference count has the responsibility of
freeing the object when the reference count reaches zero.
In addition, the code that uses a reference-counted object has
to be carefully written so that whenever a pointer is created
the method to increase the reference count is called, and whenever
a pointer is discarded the method to decrease the reference count
is called.
For mailboxes, the function `mb_ref()` is used to increase the
reference count and the function `mb_unref()` is used to decrease
the reference count.

The second issue that mailboxes have to deal with is how a mailbox
service thread that is watching the mailbox can be notified when
the mailbox is removed from the directory and can no longer be used.
For this, we use the `mb_shutdown()` function.  This function sets
a flag to mark the mailbox as in the "defunct" state.
It then has to arrange for the mailbox service thread possibly
blocked in `mb_next_entry()` to return from that call, so it can
notice that the mailbox is now defunct.
Note that the normal way of arranging for this thread
(the mailbox "consumer") to block waiting for an entry in the mailbox
would be to call `sem_wait()` on a semaphore.
When a mailbox is shut down, `sem_post()` on that semaphore can be
used to release the consumer thread so that it can return from
`mb_next_entry()`.  It is then the responsibility of the caller
of `mb_next_entry()` to check whether the mailbox is defunct and
if so, to take action to free resources and terminate.

The third issue that mailboxes have to deal with is what to do with
mailbox entries that remain when a mailbox is shut down.
The specification of the Bavarde server requires that a bounce notice
be sent to the sender of each message that is discarded because
its recipient logged out or disconnected before the message could
be delivered.  So as part of the process of finalizing a mailbox
(performed out of `mb_unref()` when the reference count reaches zero),
these bounce notices have to be generated.
In order to avoid having the mailbox module depend on the details
of its client modules, a mailbox provides the ability for its client
to provide a **discard hook** to be called when a mailbox entry is
discarded.  This is done by the `mb_set_discard_hook()` function.
During the process of mailbox finalization, if a discard hook has
been set, then it is invoked on each mailbox entry that remains
in the mailbox.

In the Bavarde server, the discard hook for a mailbox will be set
to a function that generates a bounce notice if the entry being
discarded is an undelivered message.  To do this, the discard hook
needs access to the mailbox of the original sender of a message.
To avoid excessive copying of the sender's handle and repeated
lookups of this handle in the directory, we have chosen for message
entries in a mailbox to record the sender of a message not by its
handle, but by a pointer to the sender's mailbox.  This makes it
easy to send a bounce notice, but note that a reference count must
be associated with the mailbox pointer stored in a message.
This means that when a message is discarded during mailbox finalization,
the `mb_unref()` function must be called on the `from` field of each
message being discarded.

### Task VI: Service Thread Functions

If you've made it this far, there is only one more module to implement
to complete your version of the server.
As described in the overview, each time the server accepts a connection
from a client a new thread is created to run the `bvd_client_service()`
function, and when a client logs in a new thread is created to run
the `bvd_mailbox_service()` function.  Your final task is to implement
these thread functions, which are specified in the header file
`include/server.h`.  You should implement these functions in the
file `src/server.c`.  Note that the number of lines of code required
here is probably the largest of any of the modules you have to
implement, so you should certainly define helper functions as appropriate.

> :nerd: We strongly suggest that you do not attempt this part of the
> assignment until you have successfully completed the other parts.
> You will need a good understanding of all the other parts of the
> server in order to code the service thread functions.

The `bvd_client_service()` function should contain a service loop
that repeatedly reads a request packet sent by the client, carries out
the request, and arranges for the required `ACK` or `NACK` to be sent.
At this point, you should probably have a reasonably good understanding
of what has to be done in order to carry out each request.
The `bvd_mailbox_service()` function should contain a service loop
that repeatedly calls `mb_next_entry()` on the client's mailbox.
Each entry will contain a notice or message to be sent over the wire
to the client.  In addition, when a message is sent, a delivery
notification needs to be generated and enqueued in the sender's
mailbox.

To avoid having the client service thread and mailbox service thread
collide with each other when sending packets to the client (packets do
not get written to the wire atomically and if two threads try to send
a packet concurrently the contents could get interleaved) instead of
sending `ACK` or `NACK` directly to the client, these should always be
added as notices to the client's mailbox, where they will be
discovered by the mailbox service thread and sent to the client.  The
only situation in which the client service thread should directly send
a packet to the client is if the client is not currently logged in.
This can occur in the case of a `NACK` sent in response to a login
request for a handle that is already in use, an `ACK` sent in response
to a `USERS` request from a client that is not logged in, and a `NACK`
sent in response to a `SEND` request from a client that is not logged
in.  In these situations, the client is not logged in, so there is no
mailbox in which the notice can be queued.  Happily, this also means
that there is no mailbox server thread, so it is safe in these cases
for the client service thread to send the notice directly.

## Extra Credit

There is one extra credit opportunity at the moment.  It is possible
that some others will be devised, but this is all there is right now.

### More Concurrency in Directory Access (10 points)

The simplest way to synchronize the directory is using mutual exclusion.
However, the directory functions are naturally divided into "readers"
and "writers": `dir_register()` and `dir_unregister()` are writers,
and `dir_lookup()` and `dir_all_handles()` are readers.
In a realistic setting, it is likely that there will be many more calls
to `dir_lookup()` than there will be to `dir_register()` and `dir_unregister()`,
so it would be useful to allow calls to `dir_lookup()` to proceed
concurrently.

To receive this extra credit, your directory implementation should use
the readers/writers paradigm for synchronization, rather than the
less-general mutual exclusion paradigm.  See **Section 12.5 (Page
1006)** of your textbook for more information on readers and writers.

### **Letting Us Know About Your EC**

To attempt extra credit for generalizing the directory implementation,
we need to know about it.  For this, you should create a file
`EXTRA_CREDIT` in the `hw5` directory.  This file should contain the
keyword `READERS_WRITERS` on a single line.  For this extra credit, we
will only be testing your implementation with the directory code that
you supply.  If you attempt the extra credit, make sure that it works
properly, because if it doesn't it will negatively impact your
non-extra-credit grade.

## Submission Instructions

Make sure your hw5 directory looks like this and that your homework compiles:

```
hw5
├── client
│   └── client
├── include
│   ├── debug.h
│   ├── directory.h
│   ├── mailbox.h
│   ├── protocol.h
│   ├── server.h
│   └── thread_counter.h
├── lib
│   └── bavarde.a
├── Makefile
├── src
│   ├── directory.c
│   ├── mailbox.c
│   ├── main.c
│   ├── protocol.c
│   ├── server.c
│   └── thread_counter.c
└── tests
    └── bavarde_tests.c
```

Note that you should omit any source files for modules that you did not
complete, and that you might have some source and header files in addition
to those shown.  You are also, of course, encouraged to create Criterion
tests for your code.

It would definitely be a good idea to use `valgrind` to check your program
for memory and file descriptor leaks.  Keeping track of allocated objects
and making sure to free them is one of the more challenging aspects of this
assignment.

To submit, run `git submit hw5`.

Project History
===============


CHARLIE v2.5 alpha
------------------

- First version published on the Internet via GitHub
- Features many improvements inspired by my first experiences with
  working with the Linux kernel
- This is still an alpha version and some code has yet to be revised and
  merged back in; reliability should still be considered suspect.
- Brought back RS232 debugging support
- Greatly improved the Web interface graphics and the overall Web page
  design
- Dropped the `stream` abstraction; packets are now handled using a
  `PacketBuf` structure strongly inspired by the `skt_buf` abstraction in
  the Linux kernel. As before, packet data is stored exclusively in
  ENC28J60 memory, but the `skt_buf`-like semantics allow for much more
  efficient and natural packet assembly.
- Networking is now push-based - the top-level networking layer forwards
  ('pushes') any received packed directly to the appropriate client
  in the lower layers. Outgoing packets are assembled from the inside
  out, as is the case for the Linux kernel.
- As a result of the significant paradigm changes, as well as a careful
  review of each and every function, network code is now considerably
  smaller and more robust. However, file transmission is still quite
  slow, and TCP behavior still slightly dubious when viewed in
  Wireshark.
- Implemented a 'delayed work' scheduling facility akin to that in the
  Linux kernel. This allows for much more elegant handling of timeout
  code, as well as the temporal orchestration of the core plant
  monitoring and watering functions.
- Various improvements have been made concerning the efficient use of
  RAM and Flash program space, as well as the proper modularization
  and decoupling of functional layers
- The code and commenting style has been revised so as to resemble
  `checkpatch.pl` standards
- Ports, pins, timeouts, etc. are now configured from a single header
  file in a fashion reminiscent of the `.config`->`autoconf.h` system used
  in the Linux kernel
- A battery-backed onboard RTC is now available; SNTP support has been
  dropped. The device is no longer dependent on a network connection,
  though it is still not quite portable due to the powering issues.


CHARLIE v2.0 beta
-----------------

- Presented as my term project for the Microprocessor-based Design
  course at the Polytechnic University of Bucharest
- Added Ethernet support and a web interface that replaced the
  RS232-based one; the interface featured XSLT-based dynamic web pages
  that allowed browsing of the current device status, log events,
  plant recordings, and a configuration/control screen.
- Debugging was also done exclusively via Ethernet (raw Ethernet
  packets would be sent for programs like tcpdump of Wireshark to pick
  up)
- Switched to an ATMEGA32 to accomodate the greatly increased code size
  and memory requirements
- Used an elegant "stream" abstraction to cleanly handle SD card files,
  network packets and dynamic web content under one unified datatype
- Network stack was pull-based, i.e. services such as ARP, TCP, etc.
  were modeled as clients that would be periodically invoked and try to
  "pull" (i.e. explicitly query for) packets of interest from the core
  network layer.
- Network was autoconfigured using DHCP at bootup
- Still no onboard RTC; time was fetched at bootup from an Internet time
  server using SNTP. This allowed for power interruptions, but meant
  that the device would need to have a permanent network connection.
  The design was validated with this assumption in mind.
- Owing to code size and reliability issues, FAT32 support was dropped,
  and the SD card is now formatted using a much simpler custom
  filesystem that makes use of the fact that only a small number of
  files are ever modified by the device, and that no new files are
  ever created. Thus, each file has a fixed, preallocated and contiguous
  space allocated on the SD card, with the log and records files given
  the greatest amount of space (as they are the only ones that expand
  in size indefinitely). This means that the card cannot be read on
  a PC and has to be flashed using a custom utility.
- Still no batteries; even if they were installed they would be of
  little use as the device soaks up too much power while running a
  permanent Ethernet connection (PoE would be a much more sensible
  option); also, the polling-based design of the main loop and
  network stack makes power management difficult.
- A notable issue is the stability and reliability issues introduced by
  the extremely complex networking code. Even after numerous bug-hunting
  sessions, there still remain nigh-untraceable defects that cause
  occasional crashes and corruption when the dynamic page facility is
  stressed.


CHARLIE v1.0
------------

- Implemented the following basic features:
    - Plant humidity & ilumination monitoring
    - Automatic plant watering
    - Logging plant status to the SD card
    - Status & history queries via RS232
- Used an ATMEGA16
- There was no onboard RTC; timekeeping was done via ATMEGA under the
  assumption that the device would always be powered.
- Though planned, support for problem detection (pump failing to work,
  abnormal light levels, etc.) was not implemented.
- Had (limited) FAT32 support which meant that the onboard SD card
  could be removed and read on a PC so as to retrieve the logged data
- No batteries; could be powered via USB or via a 12V adapter (an
  extremely inefficient option, but not my idea).

# wm-sensors

A hardware monitoring library for Windows with optional lm-sensors like API

## Introduction
There are numerous applications for Windows that provide monitoring for hardware and software
resources. On the other hand, the Linux Hardware Monitoring project together with the lm-sensors
project is a solid solution for the Linux platform, which collects open sourced hardware monitoring
code and provides a unified API for userspace applications. Windows solutions are either
hardware vendor-specific (closed sourced with no API for external users), closed sourced, or use
variety of APIs and implementations (C#/.NET, python, etc.) which can not be easily utilized by all
client applications.

This project aims to provide an open source solution for Windows with the following goals:

 - internal sensor API is similar to that of the Linux HWMON, allowing for easy code sharing and
 borrowing between this project and Linux HWMON;
 - provide lm-sensors-compatible API for external usage (full source level compatibility);

## Status

The project is at the early development stage. Currently it provides sensors for Intel and AMD CPUs
and general memory load sensors. There are two applications to read sensor values: the sensors app
from the lm-sensors project (no changes except compilation fixes for Windows) and a basic GUI app
(wsensors) that displays the sensors tree and their values. Both apps currently barely serve as
testing applications for the sensors library. See [CONTRIBUTING.md](CONTRIBUTING.md) for information
how to build the project.

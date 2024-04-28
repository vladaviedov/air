# AIR Protocol (Revision 1.0 Draft)

## 1. Introduction

The Autonomous Intersection Regulation (AIR) protocol is a digital radio protocol modulated with GFSK. It is designed for an automated traffic negotiation system at traffic intersections between a control station and multiple cars.

## 2. RF Protocol

### 2.1 Modulation

Transmissions will modulate ASCII data with 2GFSK at 9600 bits per second and a 28.8 kHz deviation. The protocol is best suited for UHF operation.

### 2.2 Time Division

The AIR protocol is used on a single channel with a time-divison multiple access scheme. Continuous frames are used, and the control device is the authority over time synchronization. Each frame is split into a number of 20 millisecond timeslots, which follow the following format:

| Time | Description |
|---|---|
| 0.00-12.5 ms | Transmission |
| 12.5-20.0 ms | Guard interval |

The maximum amount of data transferred in a single timeslot is 15 bytes.

For synchronization purposes, during the `CHK` operation (see section 3.2.1), the control station should measure the offset from the expected transmission starting point and the actual transmission start. It will report the offset with the next message in order to correct further transmissions.

The timeslot order follows the same pattern as the in Time division scheme intersection positions (see section 3.1.1). Clients must be told which division scheme the current intersection implements.

#### 2.2.1 Division Schemes

| Scheme | Code | Timeslots | Frame Duration |
|---|---|---|---|
| A | 0 | 4 | 50 ms |
| B | 1 | 8 | 100 ms |
| C | 2 | 16 | 200 ms |

## 3. Data Protocol

### 3.1 Definitions

| Term | Description | Notes |
|---|---|---|
| Car/Control ID | String of characters specifying station ID.<br/>Consists alphanumeric characters and special characters `-` and `/`.<br/>Maximum 12 characters.<br/>May not begin with "UN" | |
| Protocol Version | String specifying protocol used. Protocol number prefixed with `AIRv`. | For this revision: `AIRv1.0`. |
| Position | Character representing a hexadecimal digit.<br/>Index given to physical intersection entrance.<br/>Assigned counter-clockwise around the intersection. | Provided at intersection.

The command type is 3 bytes long and may hold the following data:

| Command | Description |
|---|---|
| `GRQ` | Go as requested: fulfills request. |
| `GTn` | Go to position `n`.<br/>`n` is sent as an unsigned byte. |
| `SBY` | Standby for further instructions. |

#### 3.1.1 Positions

Positions are assigned in counter-clockwise order for the intersection. Values from 0 to 15 are supported, meaning that intersection with over 16 entrances cannot utilize this protocol.

When entering the intersection, the car will obtain the following information about its position. The method of transferring this data is implementation-dependent.

| Description | Length | Type | Notes |
|---|---|---|---|
| Current position index | 1 | unsigned byte | From 0 to 15 |
| Total positions | 1 | unsigned byte | From 1 to 16 |
| Channel frequency | 8 | unsigned 64-bit | In kilohertz |
| Time division scheme | 1 | unsigned byte | Scheme code (see section 2.2.1) |
| Failure resolution behavior | 3 | command | `GTn` command |


### 3.2 Procedure

The negotiation is divided into three parts:

- Check-in
- Request
- Clearing

Whitespaces are significant and data are case-insensitive.


#### 3.2.1 Check-In

The car sends a check-in message:

| Data | Length (bytes) | Type |
|---|---|---|
| Protocol Version | 7 | string |
| Whitespace | 1 | char |
| `CHK` | 3 | string |
| Unused | 4 | N/A |

If the protocol is supported by control, it responds with identification and timing offset. The car will adjust its timings using offset information.

| Data | Length (bytes) | Type |
|---|---|---|
| Control ID | 12 | string |
| Whitespace | 1 | char |
| Offset | 1 | signed byte |
| Unused | 1 | N/A |

If the protocol version is unsupported, the control will respond with identification and not supported message in reverse order.

| Data | Length (bytes) | Type |
|---|---|---|
| `UN` | 2 | string |
| Whitespace | 1 | char |
| Control ID | 12 | string |

If the check-in message does not begin with a protocol specifier, the control shall ignore it.

#### 3.2.2 Request

The car sends identification, current and requested positions.

| Data | Length (bytes) | Type |
|---|---|---|
| Car ID | 12 | string |
| Whitespace | 1 | char |
| Current Position | 1 | unsigned byte |
| Desired Position | 1 | unsigned byte |

For valid requests, control will acknowledge and send a command.

| Data | Length (bytes) | Type |
|---|---|---|
| `ACK` | 3 | string |
| Whitespace | 1 | char |
| Command | 3 | command |
| Unused | 8 | N/A |

The car will confirm the command with an acknowledge and execute. If the car is placed in standby, it's implied that control will send another command.

| Data | Length (bytes) | Type |
|---|---|---|
| Command | 3 | command |
| Unused | 12 | N/A |

The second command must also received an acknowledgement.

#### 3.2.3 Clearing

Once the car has left the intersection, it shall send a clear message.

| Data | Length (bytes) | Type |
|---|---|---|
| `CLR` | 3 | string |
| Unused | 12 | N/A |

Upon receiving the message, control will respond with a message indicating that the channel is clear for the next car.

| Data | Length (bytes) | Type |
|---|---|---|
| `FIN` | 3 | string |
| Unused | 12 | N/A |

# THE CONVEYOR - Automated Sorting System

This system automates the sorting process of a conveyor belt by directing packages based on NFC tags.

## Description

The conveyor system integrates with the Dolibarr Warehouse Management System (WMS) to automatically route packages based on their product references stored on NFC tags. It directs packages to one of three storage warehouses while updating stock movements and tracking in real-time.

## Process Flow

1. The conveyor belt starts and moves the package forward.
2. A sensor (e.g., infrared) detects an incoming package.
3. The conveyor belt halts.
4. If the package doesn't have an NFC tag, a servo motor routes it to the error storage "B".
5. If an NFC UID is detected, the belt stops, and the servo motor adjusts its direction based on the UID, either to "C" or "D".
6. The conveyor restarts, moving the package to its designated warehouse.

## Features

- **NFC Reader**: Reads product reference from NFC tags.
- **Integration with Dolibarr WMS**: Queries WMS for product details and destination, and updates stock movements.
- **Automatic Sorting**: Directs packages to one of three warehouses based on NFC data.
- **Error Handling**: Routes packages without NFC to a designated error storage area.
- **Fast Recovery**: Quickly restores operations in case of interruptions like power outages, manual stops, or connection losses.

## Requirements

- Arduino environment with support for C/C++.
- Dolibarr WMS with stock module (communication via its REST API).
- NFC tags containing product references.
- Three warehouses designated as A, B, and C.

## Installation & Setup

1. Clone this repository.
2. Connect the hardware components as detailed in the controller documentation.
3. Configure the API connection to Dolibarr WMS.
4. Deploy the firmware to the controller.

## Testing

Testing phases include:

- **Handling of the Controller** (November): 30 minutes optional session for controller manipulation.
- **WMS Communication Tests** (December): 30 minutes optional session to validate Dolibarr integration.
- **Firmware Testing & Debugging** (January): Mandatory 45 minutes session.

## Delivery Timeline

- October: Submit intermediary report identifying controller elements.
- November to January: Optional testing sessions.
- February: Final delivery of firmware and documentation.

## Contributing

This project is currently under development. Contributions, suggestions, and feedback are welcome.

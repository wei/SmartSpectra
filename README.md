# SmartSpectra SDK

This repository hosts SmartSpectra SDK from PresageTech for measuring vitals such as pulse, breathing, and more using a camera. The SDK supports multiple platforms, including Android, iOS, and C++ for Mac/Linux.

## Table of Contents

- [SmartSpectra SDK](#smartspectra-sdk)
  - [Table of Contents](#table-of-contents)
  - [Getting Started](#getting-started)
  - [Features](#features)
  - [Authentication](#authentication)
  - [Platform-Specific Guides](#platform-specific-guides)
    - [Android](#android)
    - [iOS](#ios)
    - [Mac/Linux](#maclinux)
  - [Troubleshooting](#troubleshooting)
  - [Known Bugs](#known-bugs)

## Getting Started

To get started, follow the instructions for one of our currently supported platforms. Each platform has its own integration guide and example applications to help you get up and running quickly.

## Features

- Measure vitals such as pulse and breathing using a camera.
- Support for multiple platforms: Android, iOS, and C++ for Mac/Linux.
- Easy integration with detailed platform-specific guides.
- API key and Oauth authentication support.
- Supports headless mode in C++ and iOS (Android support coming soon).

## Authentication

You can obtain an API key from PresageTech's developer portal (<https://physiology.presagetech.com/>). We also support Oauth authentication for iOS and Android. Refer to the respective platform's README for more details.

## Platform-Specific Guides

### Android

For Android integration, refer to the [Android README](android/README.md). The guide includes:

- Prerequisites and setup instructions.
- Integration steps for your app.
- Example usage and troubleshooting tips.

### iOS

For iOS integration, refer to the [iOS README](swift/README.md). The guide includes:

- Prerequisites and setup instructions.
- Integration steps for your app using Swift Package Manager.
- Example usage and troubleshooting tips.

### Mac/Linux

For C++ integration on Mac/Linux, refer to the [C++ README](cpp/README.md). The guide includes:

- Supported systems and architectures.
- Installation and build instructions.
- Example applications and troubleshooting tips.

## Troubleshooting

For additional support, contact <support@presagetech.com> or [submit a GitHub issue](https://github.com/Presage-Security/SmartSpectra/issues).

## Known Bugs

- Currently, there are no known bugs. If you encounter an issue, please contact support or report it.

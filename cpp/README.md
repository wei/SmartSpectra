# Smart Spectra C++ SDK

This repository hosts a private SDK for measuring heart and respiration rates for use in C++ applications, along with usage examples.

## Table of Contents

 - [Quick Start](#quick-start)
 - [Supported Systems & Architectures](#supported-systems--architectures)
 - [Installing Prebuilt SDK Packages from PPA (Ubuntu / Linux Mint)](#installing-prebuilt-sdk-packages-from-debian-repository-ubuntu--linux-mint)
   - [Setting up the Presage PPA](#setting-up-the-presage-ppa)
   - [Installing/Upgrading the Smart Spectra C++ SDK](#installingupgrading-the-smart-spectra-c-sdk-debian-package)
   - [Installing Only Smart Spectra C++ SDK Dependencies](#installing-only-smart-spectra-c-sdk-dependencies)
   - [Uninstalling all Presage Deb Packages and Removing the PPA](#uninstalling-all-presage-deb-packages-and-removing-the-ppa)
 - [Examples](#examples)
 - [Developing Your Own Smart Spectra C++ Application](#developing-your-own-smart-spectra-c-application)
   - [Setting Up CMake](#setting-up-cmake)
   - [Using a Custom OnCoreMetricsOutput Callback](#using-a-custom-oncoremetricsoutput-callback)
   - [Using a Custom OnEdgeMetricsOutput Callback](#using-a-custom-onedgemetricsoutput-callback)
   - [Using a Custom OnVideoOutput Callback](#using-a-custom-onvideooutput-callback)
  - [Bugs & Troubleshooting](#bugs--troubleshooting)

## Quick Start


Obtain API key from <https://physiology.presagetech.com>.
Then, on Ubuntu 22.04 or Mint 21, follow the steps below.

### Install:

1. `curl -s "https://presage-security.github.io/PPA/KEY.gpg" | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/presage-technologies.gpg >/dev/null`
2. `sudo curl -s --compressed -o /etc/apt/sources.list.d/presage-technologies.list "https://presage-security.github.io/PPA/presage-technologies.list"`
3. `sudo apt update`
4. `sudo apt install libsmartspectra-dev`

### Run:

1. `rest_continuous_example --also_log_to_stderr --camera_device_index=0 --auto_lock=false --api_key=<YOUR_API_KEY_HERE>` 

## Supported Systems & Architectures

We currently publicly provide SDK dependency packages only for **Ubuntu 22.04 and Mint 21 Linux** distributions running on the **amd64/x86_64** architecture, but we already support many other systems and architectures for our partners, and plan to release these publicly in the near future. 

| OS                    | Package Type  | Architecture | Support Level / Status                                                                   |
|-----------------------|---------------|--------------|------------------------------------------------------------------------------------------|
| Ubuntu 22.04/Mint 21  | Debian        | `amd64`      | [Public package released](#installing-prebuilt-sdk-packages-from-ppa-ubuntu--linux-mint) |
| Ubuntu 22.04/Mint 21  | Debian        | `arm64`      | Package available to partners, release planned                                           |
| Ubuntu 24.04/Mint 22  | Debian        | `amd64`      | From-source builds in-progress, package planned                                          |
| Debian 11 & Debian 12 | Debian        | `amd64`      | Package available to partners, release planned                                           |
| Debian 11 & Debian 12 | Debian        | `arm64`      | Package available to partners, release planned                                           |
| RHEL 9/Fedora 41      | RPM           | `x86_64`     | Contact support                                                                          |
| RHEL 9/Fedora 41      | RPM           | `aarch64`    | Contact support                                                                          |
| macOS                 | PKG           | `arm64e`     | From-source builds available, package planned                                            |
| macOS                 | HomeBrew Cask | `arm64e`     | From-source builds available, package planned                                            |
| Windows               | Windows       | `x86_64`     | Contact support                                                                          |

If you're interested in partnering with us or seeing higher support levels for specific packages, please reach out to <[support@presagetech.com](mailto:support@presagetech.com)>.

### Authentication

You'll need a Presage Physiology API key to use the SmartSpectra SDK. You can register for an account and obtain an API key at <https://physiology.presagetech.com>.

## Installing Prebuilt SDK Packages from Debian Repository (Ubuntu / Linux Mint)

### Setting up the Presage Debian Repository
You'll want to set up the Presage Debian Repository to be able to install and update the Smart Spectra C++ SDK. You'll only need to do this once on your system.

Run the following commands in your terminal.
1. Download the GPG key:
    ```bash
    curl -s "https://presage-security.github.io/PPA/KEY.gpg" | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/presage-technologies.gpg >/dev/null
    ```
2. Copy the PPA list:
    ```bash
    sudo curl -s --compressed -o /etc/apt/sources.list.d/presage-technologies.list "https://presage-security.github.io/PPA/presage-technologies.list"
    ```

### Installing/Upgrading the Smart Spectra C++ SDK Debian Package

1. Update the `apt` database:
    ```bash
    sudo apt update
    ```
2. Install (or upgrade) the SDK:
    ```bash
    sudo apt install libsmartspectra-dev
    ```

### Uninstalling all Presage Deb Packages and Removing the Presage Debian Repository

1. Uninstall the SDK & other Presage packages:
    ```bash
    sudo apt remove libphysiologyedge-dev libsmartspectra-dev
    ```
2. Remove the repository:
    ```bash
    sudo rm /etc/apt/sources.list.d/presage-technologies.list
    ```
3. Remove the GPG key:
    ```bash
    sudo rm /etc/apt/trusted.gpg.d/presage-technologies.gpg
    ```
4. Update the `apt` database:
    ```bash
    sudo apt update
    ```
## Examples

You can find example C++ applications with descriptions and a more detailed walkthrough in the [samples](samples) directory.

## Developing Your Own Smart Spectra C++ Application

To begin developing your own Smart Spectra C++ application, we encourage you to start from the [minimal spot example](samples/minimal_rest_spot_example) and borrow from the [Rest Continuous Example App](samples/rest_continuous_example) as needed.

Please refer to the [Installing Build Tools](docs/build_linux.md#installing-build-tools) section of the build instructions for how to set up your environment.

### Setting Up CMake

Use the following CMake code for reference on how to set the `CMakeLists.txt` file for your application:

```CMake
cmake_minimum_required(VERSION 3.27.0)
project(MyProject CXX)

find_package(SmartSpectra REQUIRED)

add_executable(my_app main.cc)

target_link_libraries(my_app
        # Required: container for running the node graph; also links the video capture utilities 
        SmartSpectra::Container
        # Optional: for using the OpenCV-based HUD/GUI components we provide
        SmartSpectra::Gui 
)
```

### Using a Custom OnCoreMetricsOutput Callback

Typically, you will want to do something with the vitals data output from SmartSpectra & Physiology Core API.
The following code demonstrates how you can add your own callback to process and/or display the data:
```C++
container.OnCoreMetricsOutput = [](const presage::physiology::MetricsBuffer& metrics, int64_t timestamp_microseconds) {
    LOG(INFO) << "Got metrics from Physiology REST API: " << metrics;
    return absl::OkStatus();
};
```
You can see how the callback is used to format metrics as a JSON string in the [minimal_rest_spot_example app](samples/minimal_rest_spot_example/main.cc), how it is output to a file in the [rest_spot_example app](samples/rest_spot_example/main.cc), and how it is used to plot vitals data in real time in [rest_continuous_example app](samples/rest_continuous_example/main.cc).

### Using a Custom OnEdgeMetricsOutput Callback

Some metrics (namely the upper and lower breathing trace) are now available for computation on edge on a framerate basis. To enable this behavior, you need to set `enable_edge_metrics` to true in the `Settings` object you use to initialize the container (see [rest_continuous_example app](samples/rest_continuous_example/main.cc) for an example).
The following code demonstrates how you can add your own callback to process and/or display this data:
```C++
container.OnEdgeMetricsOutput = [](const presage::physiology::Metrics& metrics) {
    LOG(INFO) << "Computed metrics on edge: " << metrics;
    return absl::OkStatus();
}
```

### Using a Custom OnVideoOutput Callback

If you need to draw something on top of the video stream or forward it to some form of GUI, you can do this by setting the `OnVideoOutput` callback.

```C++
container.OnVideoOutput = (cv::Mat& output_frame, int64_t timestamp_milliseconds) {
    cv.imshow("Video Output", output_frame);
    return absl::OkStatus();
};
```
You can find how this callback is used to plot corresponding vitals data in real time directly over the video output in [rest_continuous_example app](samples/rest_continuous_example/main.cc).


More examples, tutorials, and reference documentation are coming soon!

## Building the SDK

See the [Building & Packaging on Ubuntu / Linux Mint](docs/build_linux.md) section for more details.
 - Parties that are interested in using C++ SDK on macOS: please refer to [Building SmartSpectra SDK on macOS](docs/build_macos.md). 

## Bugs & Troubleshooting

For additional support, contact <[support@presagetech.com](mailto:support@presagetech.com)> or [submit a GitHub issue](https://github.com/Presage-Security/SmartSpectra/issues)

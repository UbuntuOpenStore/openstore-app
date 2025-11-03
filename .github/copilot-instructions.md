# OpenStore App Development Guide

## Project Overview
OpenStore App is the Ubuntu Touch client for the [OpenStore](https://open-store.io/), an alternative app store for Ubuntu Touch devices. The app allows users to browse, search, install, and update applications from the OpenStore repository.

## Architecture

### Core Components
- **C++ Backend**
  - `PlatformIntegration`: Singleton that provides system information and access to the Click installer
  - `ClickInstaller`: Manages app installation/removal on the device
  - `OpenStoreNetworkManager`: Handles API requests to the OpenStore backend
  - `PackagesCache`: Caches app data to improve performance

### Data Models
- `DiscoverModel`: Featured apps on the discover page
- `SearchModel`: Search results
- `CategoriesModel`: App categories
- `LocalPackagesModel`: Installed apps

### QML Frontend
- `Main.qml`: Entry point for the UI, handling navigation and URL parsing
- Page components:
  - `DiscoverPage.qml`: Home screen with featured content
  - `CategoriesPage.qml`: App categories
  - `SearchPage.qml`: Search functionality
  - `AppDetailsPage.qml`: Remote app details
  - `AppLocalDetailsPage.qml`: Local app details
  - `InstalledAppsPage.qml`: User's installed apps
  - `SettingsPage.qml`: App settings

### Plugins
- **PamAuthentication**: QML plugin for authenticating users with their system password

## Build System
- **[Clickable](https://clickable-ut.dev/en/latest/llms-full.txt)**: Primary tool for building and deploying Ubuntu Touch apps
- **CMake**: Used for compilation and dependency management
- **Build Requirements**:
  - libclick-0.4-dev
  - libpam0g-dev
  - ubuntu-sdk-20.04 framework

## Development Workflow

### Setup
```bash
# 1. Install clickable
# Follow instructions at https://clickable-ut.dev/en/latest/install.html

# 2. Initialize submodules
git submodule update --init --recursive

# 3. Build and run on device/emulator
clickable
```

### Common Tasks
- **Building for desktop**: `clickable desktop`
- **Debugging**: `clickable logs`
- **Translation**: Contribute via [Weblate](https://hosted.weblate.org/projects/open-store-io/openstore-app/)

## API Integration
- OpenStore API endpoints are defined in `src/apiconstants.h`
- Main API base URL: `https://open-store.io/`
- The app uses v3 and v4 API endpoints for different functionality

## Project Conventions

### UI Patterns
- Follows Ubuntu/Lomiri UI guidelines using Lomiri Components
- Bottom edge navigation for main app sections
- App details follow a consistent layout pattern

### Code Organization
- C++ backend in `src/` directory
- QML frontend in `qml/` directory
- Component reuse via the `qml/Components/` directory
- Translations in `po/` directory

### Best Practices
- Use singleton patterns for core services (NetworkManager, PackagesCache, PlatformIntegration)
- Respect the model-view separation between C++ models and QML views
- Handle connectivity issues gracefully with offline mode support

## Issues and Bug Reports
Report bugs at https://gitlab.com/theopenstore/openstore-meta/issues

# TankSafe - Fuel Theft Monitoring and Alert System

## Overview

TankSafe is an IoT-based solution designed to monitor fuel levels in vehicles and provide real-time alerts in case of fuel theft or unauthorized siphoning. By leveraging advanced sensors, cloud-based data storage, and GPS integration, TankSafe helps logistics and transportation companies prevent fuel theft, optimize fuel consumption, and enhance overall fleet management efficiency.

## Features

- **Real-Time Fuel Monitoring:** Track fuel levels continuously, ensuring that any abnormal decrease is instantly detected.
- **Instant Alerts:** Notifications sent via SMS and mobile app to alert the fleet manager of potential fuel theft.
- **Data Analytics:** Detailed reports and analytics to monitor fuel efficiency, consumption patterns, and overall performance.
- **GPS Integration:** GPS tracking to locate vehicles in real-time and ensure fuel levels are correlated with travel.
- **User-Friendly Interface:** Web and mobile dashboards for managing fleet fuel data, available anytime and anywhere.
- **Multi-Vehicle Support:** Scalable solution capable of handling fleets of any size.

## System Architecture

### 1. IoT Sensors
- Fuel level sensors installed in vehicles measure fuel levels in real-time.
- Sensors are connected to an IoT device (e.g., ESP32) which transmits data to the cloud.

### 2. Cloud Integration
Sensor data is sent to a cloud-based platform for storage and analysis. **Atharv Sharma** led the design and development of the cloud infrastructure, ensuring seamless integration and real-time data synchronization.

### 3. Web Dashboard
A web interface provides an overview of the entire fleet's fuel levels, alerts, and analytics. Built by **Srinjoy Das**, the web dashboard ensures ease of use and allows fleet managers to make informed decisions based on fuel data.

### 4. Mobile Application (to be added)
The mobile app provides real-time notifications and data access on-the-go. Developed by **Debya Debashish Bhoi**, the app ensures fleet managers are immediately alerted in case of fuel theft, and the user experience is enhanced through thoughtful UI/UX design.

### 5. Alerts and Notifications (to be added)
- Instant alerts are generated when fuel levels drop unexpectedly, ensuring timely intervention.
- Notifications are sent to the fleet manager via SMS and the TankSafe mobile app.

## Contributors

- **Atharv Sharma**: IoT, Hardware, and Cloud Integration  
  Atharv designed the IoT network and integrated cloud storage, ensuring real-time data synchronization and secure fuel monitoring.

- **Srinjoy Das**: Web Developer  
  Srinjoy developed the web dashboard for fleet managers, providing them with a comprehensive and user-friendly interface to monitor fuel levels, generate reports, and manage alerts.

- **Debya Debashish Bhoi**: App Developer and UI/UX Designer  
  Debya designed and developed the mobile app with a seamless user experience, providing fleet managers with real-time alerts and remote access to fuel data.

# Metric System

## Initial Requirements
- Relatively larger company with tens of thousands of hosts
- We want to see how services and machines are performing
- What metrics are we concerned about:
  - Monitoring the health of our systems:
    - CPU (at this second, the machine is using 80% of CPU capacity), memory, network I/O, disk I/O
    - GC latency, out-of-memory errors, number of threads
    - How long are requests taking, how often are requests failing
- At what interval are we pulling system statistics
  - Every second is OK (every 10sec probably still OK)
- On latency, if something fails, users should be alerted within 10 secs of the failure ocurring so that they get to solve the issue quickly
  - If there's like a 2min lag users will be unhappy since its a pretty urgent situation
- Out of scope: logging errors / exceptions
- Consistency requirements:
  - It doesn't have to be perfect, but it shouldn't be "biased" (like you shouldn't get a false sense of security from your data)
- Our system will have a dashboard where all metrics will be displayed.

## Assumptions
- We're retaining metrics data in our data store for one year.
- The number of hosts sending data is on the upper end of the scale, say 750,000 hosts.
- Each machine inside the company network has an agent installed that pushes data to a collector.
- The same number of machines will be accessing the metrics dashboard. User will check the dashboard once every hour.

## What are we dealing with?
- Each machine is sending or pushing the following data points (via the agent) to our collector:
  - CPU load
  - Memory usage
  - Network I/O
  - Disk I/O
  - GC latency
  - OOMs
  - Number of threads
  - Response time
- Each metric is sent separately.
- We have something called time series data, which is basically data at a specific point in time. Each data point has a timestamp which represents the time when the metric was collected.
- This is a write-heavy application as we are literally writing tens of thousands of data points per second.

## What components do we need?
- Metrics source: machines, services, clusters, etc.
- We can use a service such as OpenTelemetry Collector to collect metrics from different sources.
- A data store to store these metrics.
- A dashboard to allow users to query metrics.

## Data model
- A record for a metric could look like this:
```
{
  "metric_name": "memory.usage", // string -> 20 bytes
  "timestamp": "1691622800", // int64 -> 8 bytes
  "value": "35" // int64 -> 8 bytes
}
```
- Total: 36 bytes

## Calculations
- Volume: 750,000 data points * 36 bytes = 27,000,000 bytes = 27 MB
- Hourly storage: 27 MB * 3600 seconds = 97,200 MB = 9.72 GB/hour
- Daily storage: 9.72 GB/hour * 24 hours = 2,332.8 GB/day = 2.3 TB/day

## Storage
Writing all this data to a SQL database is probably not a idea since the DB would require heavy expertise for tuning it for these types of workloads. It is probably the same case with NoSQL DBs. Instead, we can use stores that exist specifically for dealing with time-series data (e.g., [InfluxDB](https://www.influxdata.com/lp/influxdb-database/?utm_source=google&utm_medium=cpc&utm_campaign=Performance_Max_General&utm_content=pmax&utm_source=google&utm_medium=cpc&utm_campaign=&utm_term=&gad_source=1&gclid=Cj0KCQjwsaqzBhDdARIsAK2gqne6T4dmlJD7KHxRfHBz91_cuFBKlRQAJXdT3k5QDXRBBFneXITGK-AaArpCEALw_wcB)).


## High-Level Design
1. Machines, services, and clusters push data to the collector at an interval of 1 second.
2. The collector then collects all these metrics.
3. The collector then writes data into the time-series DB.
4. A service that sits between the DB and the dashboard queries the DB and sends data back to the frontend (dashboard).
  

# Target Role & Baseline

- **Target Role**: [Cloudflare Customer Reliability Engineer (CRE) — Singapore (8317751)](https://job-boards.greenhouse.io/cloudflare/jobs/8317751?gh_jid=8317751).
- **Timeline**: **3 months (13 weeks)** immediately following [Curriculum.md](Curriculum.md), from **5 Apr 2028 to 4 Jul 2028**.
- **Baseline You Bring from `Curriculum.md`**: Deep Linux/OS internals, sockets, `epoll`/`io_uring`, packet-capture basics, HTTP proxy internals (`proxylab`), storage/streaming engines, and performance profiling.
- **Role-Specific Gaps This Plan Closes**: L3/L4/L7 Internet protocols (**BGP, Anycast, DNS, TLS, HTTP/2 & QUIC, WebSockets, GRE/IPSec**), live packet-level debugging (`tcpdump`, `tshark`, `dig`, `mtr`, `openssl`), **Cloudflare edge/developer stack** (Workers, WAF, CDN, Zero Trust), and **AI-native incident diagnostic tooling**.

## Weekly Cadence

- **Protocol & Packet Labs (7 hrs/week)**: Hands-on network topologies (FRRouting/BGP, authoritative/recursive DNS, TLS/HTTP3/WebSockets, GRE/IPSec) + `tcpdump`/`tshark` capture analysis.
- **Tooling, Automation & AI Diagnostics (6 hrs/week)**: Python/Bash CLI diagnostic tools, PromQL/SQL/Elasticsearch log correlation, Cloudflare Workers, and AI-assisted RCA agent prototypes.
- **Live Incident & Escalation Drills (5.5 hrs/week)**: Timed cross-layer debugging ("edge -> BGP -> DNS -> TLS -> origin") and customer-ready postmortems.
- **Mock Interviews & Communication (3 hrs/week)**: Live debugging simulations, system design, and executive/customer incident communication.

---

# Month 1: L3/L4/L7 Internet Protocols & Packet-Level Mastery (Apr 5 – Apr 30, 2028)

## Week 1: Apr 5 – Apr 9, 2028 | TCP/IP, Subnetting, NAT & Packet Capture Fluency
- **Protocol Lab**:
  - Drill IPv4/IPv6 subnetting, CIDR, ARP, ICMP, TCP state machine (`SYN`, `SYN-ACK`, `FIN`, `RST`, `TIME_WAIT`, window scaling, SACK), and stateful vs stateless firewalling (`nftables`/NAT/conntrack).
  - Master `tcpdump` BPF filters, `tshark` stream/field extraction, `ss -ti`, `mtr`, and `traceroute` (ICMP vs UDP vs TCP).
- **Tooling**:
  - Write a Python/Scapy or `tshark`-backed script that automatically summarizes TCP retransmissions, resets, and RTT spikes from a `.pcap`.

## Week 2: Apr 10 – Apr 16, 2028 | BGP Routing, Anycast & Tunneling (GRE / IPSec)
- **Protocol Lab**:
  - Spin up a multi-AS BGP lab (FRRouting / `containerlab` or `netns`): practice AS-path prepending, local-pref, MED, BGP communities, route flaps, Anycast traffic steering, and diagnosing **asymmetric routing** from a single POP.
  - Configure GRE and IPSec tunnels; inspect MTU/MSS clamping and fragmentation issues (`DF` bit / PMTUD blackholes).
- **Incident Drill**:
  - Diagnose a simulated asymmetric-latency event for a financial customer using `mtr`, BGP looking-glass outputs, and packet captures.

## Week 3: Apr 17 – Apr 23, 2028 | DNS Internals (Authoritative, Recursive & DNSSEC)
- **Protocol Lab**:
  - Run an authoritative + recursive DNS pair (`bind`/`unbound` or `coredns`); trace root -> TLD -> authoritative resolution with `dig +trace`, EDNS0, CNAME flattening, negative caching (`NXDOMAIN` SOA TTL), and DNSSEC chain validation (`RRSIG`, `DNSKEY`, `DS`).
- **Tooling**:
  - Write a Python DNS health & propagation checker that compares authoritative vs recursive (`1.1.1.1`) responses and flags DNSSEC/TTL anomalies.

## Week 4: Apr 24 – Apr 30, 2028 | TLS/SSL, HTTP/1.1–HTTP/3 (QUIC) & WebSockets
- **Protocol Lab**:
  - Inspect TLS 1.2 vs TLS 1.3 handshakes, SNI, ALPN, OCSP stapling, cipher negotiation, and mTLS failures using `openssl s_client` and `tshark`.
  - Debug HTTP/2 multiplexing/HOL-blocking, HTTP/3 over QUIC (`curl --http3`), and WebSocket upgrade (`101 Switching Protocols`) + idle-timeout / proxy-buffering disconnects.
- **Incident Drill**:
  - Reproduce and isolate a recurring WebSocket disconnect caused by intermediate proxy/WAF/load-balancer timeout mismatch.

---

# Month 2: Cloudflare Edge Stack, Proactive Observability & AI Diagnostics (May 1 – Jun 4, 2028)

## Week 5: May 1 – May 7, 2028 | Cloudflare Practitioner Lab (DNS, CDN, WAF, Workers)
- **Platform Lab**:
  - Deploy a real test origin behind Cloudflare (free/dev tier): configure DNS, Cache Rules (`Cache-Control`, `CF-Cache-Status`, vary headers), WAF custom rules, rate limiting, and Zero Trust Access tunnels (`cloudflared`).
  - Build and deploy 2 **Cloudflare Workers** (plus R2/D1): an edge request-header/routing inspector and a synthetic health-check/telemetry collector.

## Week 6: May 8 – May 14, 2028 | Cross-Layer Observability (Edge + Origin Correlation)
- **Observability Lab**:
  - Stand up Prometheus + Grafana + Elasticsearch/Kibana (or Loki) + OpenTelemetry tracing; propagate `CF-Ray` / W3C `traceparent` from edge Worker through origin proxy to backend logs.
  - Drill PromQL, SQL, and Kibana/ES queries to isolate a silent WAF false-positive spike or 520/522/524/525 edge-to-origin error class across tenants.

## Week 7: May 15 – May 21, 2028 | Proactive Detectors & Diagnostic CLI Tooling
- **Automation Lab**:
  - Build a multi-layer diagnostic CLI in Python (`curl` timing breakdown, `dig` resolution, `openssl` cert/chain check, TCP connect + HTTP status matrix) that produces a single structured evidence report.
  - Implement a statistical/SLO anomaly detector (error-rate / p99 latency burn-rate) over streaming edge logs.

## Week 8: May 22 – May 28, 2028 | AI-Native Incident Pre-Diagnosis Agent
- **AI Tooling Project**:
  - Prototype an internal CRE diagnostic agent in Python: given a simulated customer ticket (symptoms, domain, timeframe), fetch relevant edge logs, DNS/TLS/BGP checks, and config diffs, then output a **cited root-cause hypothesis** and verification commands.
  - Evaluate agent accuracy and time-to-triage reduction across 5–10 synthetic incident fixtures.

## Week 9: May 29 – Jun 4, 2028 | Full-Stack Chaos & Blameless Postmortem (Buffer)
- **Capstone Drill**:
  - Run 2 timed (30-min) cross-layer chaos scenarios combining BGP/DNS/TLS/WAF/origin failures.
  - Write crisp, customer-safe incident updates alongside engineering-grade blameless postmortems (timeline, root cause, blast radius, upstream fix, and recurrence-prevention automation).

---

# Month 3: Cloudflare CRE Interview Simulation & Calibration (Jun 5 – Jul 4, 2028)

## Week 10: Jun 5 – Jun 11, 2028 | Scripting, Log-Parsing & Troubleshooting Mocks
- **Mocks**:
  - Complete 4 timed 45-minute Python/Bash coding & troubleshooting rounds: parsing multi-GB HTTP/DNS logs, concurrent endpoint probing with backoff/jitter, and live Linux debugging (`strace`, `ss`, `tcpdump`, `dig`).

## Week 11: Jun 12 – Jun 18, 2028 | Deep Networking & Architecture Mocks
- **Mocks**:
  - Whiteboard and talk through 3–4 full 60-minute scenarios:
    - Trace a packet end-to-end from a user browser through Anycast BGP, Cloudflare edge (TLS, WAF, Cache, Worker), to an origin server — and explain every possible failure mode (`520`–`526`, MTU, TLS, DNS).
    - Design a multi-tenant edge telemetry & early-warning detector pipeline.
    - Explain DDoS mitigation at L3/L4 (SYN floods, amplification, Magic Transit/GRE) vs L7 (bot/WAF/rate-limiting).

## Week 12: Jun 19 – Jun 25, 2028 | Customer-Facing Leadership & Behavioral Prep
- **Preparation**:
  - Prepare 6–8 structured STAR stories tailored to CRE: owning an ambiguous Sev-1 cross-layer outage, communicating clearly with panicked enterprise/bank stakeholders, partnering with Product Engineering on an upstream fix, and turning a recurring ticket class into automated detection/tooling.

## Week 13: Jun 26 – Jul 4, 2028 | Final Calibration & Application Launch
- **Final Polish**:
  - Light review of weak protocol/tooling spots from Weeks 10–12 mocks (no cramming).
  - Run 2 final mixed mocks (1 live networking debug + 1 architecture/AI-tooling walkthrough).
  - Align resume & GitHub portfolio (highlighting your diagnostic CLI, Workers/edge tracing lab, and AI pre-diagnosis agent) and apply for [Cloudflare Customer Reliability Engineer (8317751)](https://job-boards.greenhouse.io/cloudflare/jobs/8317751?gh_jid=8317751).

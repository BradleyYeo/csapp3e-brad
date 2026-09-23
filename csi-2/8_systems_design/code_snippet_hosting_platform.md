# [WIP] Code Snippet Hosting Platform

## Requirements
- Must support 10s of millions of daily active readers (global product)
  - Surely some snippets get really popular
  - Can’t execute code snippets
- Ideally snippets have multiple files
- Editing should be supported
- Same URL should point to new version
  - Update button is fine, no need for “auto-save”/live-editing
  - Users viewing the document will need to refresh to see new version
- Indefinite persistence of snippets
- Expect more reads than writes
- Generally snippet size is “short” but users should not feel constrained by a snippet size limit
- Do not need directory/search for this phase
- Uptime: Expect good reliability
- No accounts but somehow we need to only let creator edit the file
- Doesn’t feel like a latency-sensitive application to our product manager.
- In the future: Version History (i.e. should be obvious how to extend v1 design to support this)
- Failure Modes
  - Shouldn’t accept partial updates i.e. one file failing
  - If deleted readers should see “snippet deleted”
- We want to capture metadata about a snippet (e.g., created_at, updated_at, etc.).
- File validation/sanitation is not our problem, we shouldn't worry about this.

## Iteration #1: Single Machine
### What we'll need
Initially, we will need at the very least the following for every code snippet:

`created_at` - The time at which the snippet was created.

`updated_at` - The time at which the snippet was updated by the original author.

`snippet_uuid` - The unique identifier of the code snippet.

`author_token` - The token that grants the author permission to update the snippet.

`snippet_language` - The Programming Language in which the code snippet is written.

`snippet_text` - Actual text of the code snippet.

### Calculations

Since we have 10s of millions of daily active **readers** let's assume that we're somewhere in between the range of 10-99 million daily active users. Let's take 60 million as a middle point to avoid being too conservative. Also, since we can expect that our application will have more reads than writes, let's assume a read-to-write ratio of 10:1.

**Metadata Table**: Stores metadata about snippets:

`created_at` - 64-bit integer, 8 bytes

`updated_at` - 64-bit integer, 8 bytes

`snippet_uuid` - 16 bytes when stored as binary format

`author_token` - 16 bytes

`snippet_language` - 16 bytes (1 byte per character)

**Snippet Table**: Stores the actual snippet text:

`snippet_text` - 10,000 bytes

`snippet_uuid` - 16 bytes when stored as binary format

Total Size per Snippet: Metadata + Snippet Text: 64 bytes + 10,000 bytes = 10,064 bytes (~10 KB)

### Network and Disk Storage Calculations

Assumptions:
- 60 million daily active readers
- Read-to-write ratio: 10:1
- Each snippet is read an average of 5 times per day.

Reads:
- Total Reads per Day: 60 million readers * 5 reads = 300 million reads/day
- Data Transfer per Read: 10 KB
- Total Data Read per Day: 300 million * 10 KB = 3,000,000,000 KB (~2.8 TB/day)

Writes:
- Writes per Day: 300 million reads/day ÷ 10 = 30 million writes/day
- Data Transfer per Write: 10 KB
- Total Data Written per Day: 30 million * 10 KB = 300,000,000 KB (~280 GB/day)

Disk Storage:
- Assuming each snippet persists indefinitely, we need to account for long-term storage.
- Yearly Storage Requirement (assuming no deletions): 
  - 30 million snippets/day * 10 KB = 300,000,000 KB/day
  - Yearly: 300,000,000 KB/day * 365 days = 109,500,000,000 KB/year (~102 TB/year)
  - 
### Components to Scale
- Web Servers: Scale out to handle increased traffic.
- Database: Use a distributed database or sharding to manage the large volume of writes and reads.
- Storage System: Use a scalable object storage system (e.g., Amazon S3) to handle the large volume of snippet data.
- Cache: Use a distributed caching system (e.g., Redis) to cache frequently accessed snippets and reduce load on the database.
- Load Balancer: To distribute traffic across multiple web servers.

### Failure Modes Handling
- Atomic Updates: Ensure atomicity in updates to prevent partial updates. Transactions in the database can help here.
- Deletion Handling: When a snippet is deleted, mark it as deleted and ensure readers see a "snippet deleted" message.

### Future Considerations
- Version History: Store previous versions of snippets along with metadata changes. This could be implemented by adding a version number and storing each version's content separately.
- Scalability: As the user base grows, scale out web servers, databases, and storage. Consider using a microservices architecture to separate concerns and improve maintainability.


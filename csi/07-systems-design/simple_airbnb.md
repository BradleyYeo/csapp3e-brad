# Simple Airbnb

## Requirements Document
[https://docs.google.com/document/d/1JSi1MHRGs69XnmgkzLpuZuWaO1UZo-NBKP50lv6-qww/edit#heading=h.o1usn3j8q9ik]()

## Notes from video:
- People should be able to book a room transactionally, meaning two people can't book the same room at the same time.
- Let's assume that, for location purposes, they will zoom in/out a map. If we'd like the users to search by city, let's also assume that we can reach out to some other place, find out where New York City is (for example), and then zoom them in on that map. The map becomes the interface for the user. Assume we have some way to match cities with rectangles on the map.
  - For this, assume that we can call Google Maps' API and obtain the coordinates for a given city.
- The reservation/booking cannot be modified once it's made.
- Our design should be extensible, meaning that if we wanted to integrate a system for reviews we should be able to do it seamlessly. We shouldn't worry about reviews.
- Assume there are transactional messages (e.g., an email arrives when a booking is complete).
- Phone numbers can be used to communicate the client with the host.
- We don't need to implement an auth system, the user can already sign in with Google, Facebook, etc.
- The host can't reject a booking. If a room is available and someone books then that's final.

## Additional Assumptions
- 1 million new listings are created per year.
- 1 million places, each of them has pictures and a description.
  - Assume the host can upload 5 images of the room/property.
    - Each image has a size of say 2 MB.
  - Assume the description has a max length of 2000 characters.
- On data access: assume a Pareto distribution on users browsing vs users booking. Also, number of potential guests is significantly bigger than the number of hosts.
- Assume that if a user filters by a city in the map view, our system will make a request to Google Maps API to get the longitude and latitude for, let's say, New York. Once our system receives those coordinates, it will zoom in on the map the place will show up within the geo-rectangle for this city. Does this mean we don't need to store anything related to location in our models...?

## User Journeys
- A user can sign up as either a guest or host.
- A guest books a place.
- A host posts a place for booking.

## Data Models
- `listing`
  - listing_id - BIGINT -> 8 bytes (auto-increment) PK
  - user_id - BIGINT -> 8 bytes FK reference to `user`
  - description - VARCHAR(2000) -> 2000 * 4 = 8000 bytes
  - price - INT -> 4 bytes
  - available_from - TIMESTAMP -> 8 bytes
  - available_until - TIMESTAMP -> 8 bytes
  - CONSTRAINT available_until > available_from
- `listing_image`
  - image_id - BIGINT -> 8 bytes (auto-increment) PK
  - listing_id - BIGINT -> 8 bytes FK reference to `listing`
  - image_url - TEXT -> assume 2000 bytes
- Total bytes for a listing: 10,052 bytes (rounding up) = 11,000 bytes
---------------------------------------------------------
- `booking`
  - booking_id - BIGINT -> 8 bytes (auto-increment) PK
  - user_id - BIGINT -> 8 bytes
  - listing_id - BIGINT -> 8 bytes
  - happened_at - TIMESTAMP -> 8 bytes
  - expires_at - TIMESTAMP -> 8 bytes
  - status ENUM('free', 'booked', 'reserved') -> 4 bytes
- `user`
  - user_id - BIGINT -> 8 bytes (auto-increment) PK
  - name - VARCHAR(255) -> 255 * 4 = 1020 bytes
  - last_name - VARCHAR(255) -> 255 * 4 = 1020 bytes
  - email - VARCHAR(255) -> 255 * 4 = 1020 bytes
  - phone_number - VARCHAR(20) -> 20 * 4 = 80 bytes
  - role - ENUM('guest', host') -> 4 bytes
-----------------------------------------------------------
- Total bytes: 13,248 bytes (rounding up) = 14,000 bytes

## Calculations
- Data Storage for Listings:
  - Listings: 1,000,000 places * 11,000 bytes = 11 GB
  - Images: 1,000,000 places * 5 images/place * 2 MB/image = 10,000,000 MB = 10 TB
- Data Storage for Bookings:
  - Let's assume each listing gets booked twice a year on average.
  - Bookings per Year: 1,000,000 places * 2 bookings/year = 2,000,000 bookings/year
  - Bookings Data per Year: 2,000,000 bookings * 36 bytes (approx. size per booking) = 72 MB/year
  - Retention Policy: 7 years
  - Total Bookings Data: 72 MB/year * 7 years = 504 MB
- User Data:
  - Assume 10 million users.
  - User Data: 10,000,000 users * 1,020 bytes = 10,200,000,000 bytes = 10.2 GB
- Total Data Storage:
  - Listings: 11 GB
  - Images: 10 TB
  - Bookings: 504 MB
  - Users: 10.2 GB
- Total: ~10.22 TB/year

## Endpoints
- `GET /listings` -> Gets all available listings paginated
  - Optional query parameters. The guest can add the `sort=asc` or `sort=desc` query param to any of the following to sort results:
    - `GET /listings?price_low=<some_number>&price_high=<some_number>` -> Gets all listings within a given price range.
    - `GET /listings?date_low=<some_date>&date_high=<some_date>` -> Gets all listings that are available for booking within the specified date range.
    - `GET /listings?city=newyork` -> Gets all listings filtered by city (works on Map View).
- `GET /user/:user_id:` -> Gets a user by id.
- `POST /listing` -> Submits a new listing for a host.
- `POST /listing/:listing_id:/reserve` -> Reserves a place for 5mins.
- `POST /listing/:listing_id:/book` -> Creates a booking for a place.
- `PUT /listing/:listing_id:/price` -> Updates the price of a listing.
  - Optionally, the host can also update the price within a specific date range:
    - `PUT /listing/:listing_id:/price?date_low=<some_date>&date_high=<some_date>`
- `POST /listing/:listing_id:/release` -> Releases a listing from being in the `reserved` state.
- `POST /book` -> Confirm/create a booking.
- `POST /booking/:booking_id:` -> Send state of booking to backend.

## What high-level components do we need?
- CDN: Distributes static content globally to reduce latency.
- Load Balancer: Distributes incoming traffic to frontend servers.
- Auto-scaling Group of Frontend Servers: Scales based on traffic.
- Auto-scaling Group of Backend/API Servers: Handles business logic and database interactions.
- Cache: Stores frequently accessed data to reduce database load.
- Database: Stores user data, listings, and bookings.
- Image Storage (S3): Dedicated for storing and retrieving images. Backend servers will interact with this storage for uploading and accessing images.
- Message Queue: Handles asynchronous tasks like sending emails to the user to say that their booking is confirmed, for example.
- External Services: Integrates third-party services like Google Maps API for location services and/or Email Service, etc.

## High-Level Design
![Simple Airbnb Design](https://github.com/bradfield-csi-5/omarfq/assets/43190119/c01dcb3f-107e-4272-883f-4f9b67cb6da4)



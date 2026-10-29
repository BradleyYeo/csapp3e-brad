# Return YouTube Dislikes

## Requirements
- Want an extension that can bring back dislike counts on YouTube videos
- We have like / dislike data from ~1 billion videos from the past
- We also have fresh data from users of our extension
  - ~5 million users
- The counters of like and dislike need to be good enough so that the user can see if the video they're watching is sh*t or not

## Assumptions
- In terms of both users and videos, we're focusing on the US, not globally.
- People watch videos more than they liked or dislike
- A large degree of "eventual consistency" is fine
- After people like or dislike a video, the main thing they want to see is that the like / dislike was registered
- People should be able to rescind their like/dislike
- Do we foresee any demographic hotspots?
  - Can make simple statistical assumptions (e.g. evenly distributed)
- We don't have access to YouTube's API to get the exact number of dislikes
  - We would have to estimate the number of dislikes a video has based on likes and historical and extension users' dislikes
  - If the video was released after 2019, then the dislike data it has (and the one we can display) comes entirely from other users with the extension installed.
  - If the video was released earlier than 2019, then it has historical like/dislike data and also likely data from extension users.
- User Auth is handled by YouTube
- Some other teammate will transform the data into the format that we need (we're talking here about the historical data from older videos)
- If the user has the Extension installed, we have access to the page the user sees when he opens a video.
- Refreshing the page is fine for updating the dislike count (no live reloading).
- Out of the 5 million users that have the extension installed, only 10% like/dislike the video.

## API Definition
- GET v1/actions
- POST v1/action

## Schema

- `video_id` - 11 bytes (these IDs seem to be a variation of a base64 encoded number, source: [](https://stackoverflow.com/questions/830596/what-type-of-id-does-youtube-use-for-their-videos)
- `total_likes` - int32 4 bytes
- `total_views` - int64 8 bytes
- `extension_dislikes` - int32 4 bytes
- `extension_likes` - int32 4 bytes
- Total: 31 bytes, rounding up 40 bytes

## Calculations
Assume that out of those 5 million users who have the extension installed, only 10% actually like/dislike a video, so 500,000 users. Let's say that the average user watches two videos per day. In total, the number of videos liked/disliked daily is approx. 1,000,000.

- Storage:
1,000,000 videos * 40 byes = 40,000,000 bytes = 40 MB / day




### Notes
- If a user with the extension opens a video that he/she already disliked, the extension will take note of that dislike and add it to the estimated number of dislikes.
- If a user with the extension hits the dislike button, we need to store these somewhere to eventually update the dislikes count for this particular video.
- If a user does not have the extension installed, we need to fetch the number of dislikes from our data store to show it to the user.
- We want to use an eventually consistent data store since our system does not require a strong consistency model.

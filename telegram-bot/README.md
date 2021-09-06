# Telegram Bot

This bot is a simple message delivery bot, mostly to centralize the usage of the API, and to allow keeping track of the channels that are allowed to be sent to in a single place.

## Building

I use the build.sh file included, since I need to allow this to run on arm64 (Raspberry PI), and deploy this to my local Docker registry, so I run a multi-arch build using buildx, YMMV, there is nothing preventing this from being build using just the `docker build` command.

## Configuration

The available channels are to be provided in a JSON file located at `/telegram-bot/etc/channels.json`. The format is as follows, just a mapping from a friendly name to a channel id:

```json
{
    "channel-1": "-1001234567890",
    "channel-2": "-1002345678901",
}
```

There are a number of places on the internet where people have shared how to get the channel ids, the one that I found helpful is [here](https://gist.github.com/mraaroncruz/e76d19f7d61d59419002db54030ebe35), although it is likely to disappear or become incorrect as the Telegram web apps change - YMMV, Google is your friend.

Two environment variables are required as well:
 - CONTEXT_PATH: The context path to where this ReST interface is listening, helpful if you are deploying this behind a reverse proxy on a site with other things.
 - BOT_TOKEN: This will be custom to you, you will need your own bot - at the time of this writing, [instructions](https://core.telegram.org/bots) were available on the Telegram site.

## Usage

The API is pretty simple, just POST a JSON body to the context path, listening on port 8080, with something like the following:

```json
{
    "channel": "channel-1",
    "msg": "Hello World!",
    "silent": true
}
```

The `silent` field is just a boolean hint to the client as to whether a more aggressive notification should be made to the user, YMMV. The field is optional, and defaults to `false`.

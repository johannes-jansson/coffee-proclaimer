# Coffee-proclaimer

> Because coffee is great news!

A service to notify the office via Slack when a fresh pot of coffee is done! ☕️😎

The idea is to measure the current draw of the coffee maker. It shoud look
something like this for a typical brew:

![current graph](current_graph.png)

While the brewer is boiling the water the current draw should be at it's peak.
After the water has been boiled there should still be some current draw for the
heated pad, at this plateau we know that there is fresh coffee in the pot. When
the current draw reaches zero again somebody (or a timer) has turned of the
heated pad, and there is no more (good) coffee to be had.

The first downward slope should trigger a "The coffee is done" message in the
coffee channel. The second downward slope should notify people that there is no
more coffee to be had, they missed their opportunity 😏

This is all implemented with a non-invasive current probed clamped around the
cable to the coffee maker. This is hooked up to a wifi-connected
microcontroller, which will post to the Slack API using webhooks. 

## Parts list

- Particle photon
- Non-invasive current probe (https://www.electrokit.com/produkt/stromprob-30a/)
- 3.5mm jack
- Resistor
- (Encasing)
- (Power supply)

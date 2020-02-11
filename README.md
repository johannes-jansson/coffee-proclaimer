# Coffee-proclaimer

> Because coffee is great news!

A service to notify the office via Slack when a fresh pot of coffee is done! ☕️😎

The idea is to measure the current draw of the coffee maker. It shoud look
something like this for a typical brew:

![current graph](current_graph.png)

> ![current graph real](current_graph_real.png)
>
> Update! It actually looks like this 👆

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


## Contributing

Please contribute! Make a pull request against the `develop` branch. Custom messages (low hanging fruit 🍒) are handled in the functions `started`, `done` and `finished`, last in `app.ino`. 


## Parts list

- Particle photon
- Non-invasive current probe (https://www.electrokit.com/produkt/stromprob-30a/)
- 3.5mm jack
- Shunt resistor (357 ohms for a 2000 W coffee maker, calculations below)
- Voltage divider resistors
- Filter capacitor
- (Encasing)
- (Power supply)

![breadboard](breadboard.png)


## Electrical math

The current probe measures up to 30 A, which yields a current of 15 mA. As an
example they provided that a 10 ohm resistor provides a measurable voltage of
5 mV/A. 

The microprocessor measures voltage from 0 to 3.3 v in 4096 steps. 

The moccamaster I googled runs on 1400 W, 13 A. 

Since U = R * I, their example becomes:

```
30 A in the big cable => 15 mA in the small cable
1 A in the big cable => 0.5 mA in the big cable
10 Ohm * 0.5 mA = 5 mV
```

which checks out 👌

For our use case, let's make sure we can tolerate 2000 W, 18.5 A, in
the big cable. This corresponds to 9.25 mA in the small cablle. The
calculations for the resistor value becomes:

```
R = U / I = 3300 mV / 9.25 mA ≈ 357 Ohms
```


## Modeling math

We will assume a linear model for the amount of coffee X that is boiled in
time Y: `Y = k1 * X + m1`. Then another one for the amount of time Y it takes
for X cups of coffee to drip through the filter, after the boiling is
completed: `Y = k2 * X + m1`. Based on 4 measurements on our Moccamaster, we
end up with:

```
k1 = 36.5
m1 = -500
k2 = 34000
m2 = 11600
```


## Creds

* David Cervantes Caballero, [for his implementation of subtracting virtual ground and calculating RMS](https://scidle.com/how-to-use-non-invasive-ac-current-sensors-with-arduino/).

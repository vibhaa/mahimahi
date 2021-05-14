mahimahi: a web performance measurement toolkit

Ability to run a bursty loss shell added that transitions into a lossy state with probability `RATE1` and transitions back into a normal state with probability `RATE2`. To use it, run:
```bash
mm-loss bursty uplink|downlink RATE1 RATE2
```

In light of this, the basic random loss shell can be spawned using
```bash
mm-loss IID uplink LOSS_RATE
```

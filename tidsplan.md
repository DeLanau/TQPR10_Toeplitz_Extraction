# Time planning thesis work -- Love Arreborn, Nadim Lakrouz

The plan below is a loose plan for out thesis work.

## Update 2025-04-28

Timeplan updated with checkboxes for completed tasks.

## Risk assessment

- As it stands right now, it's unclear whether we'll be able to connect our
  device to the actual OQRNG-device before end of project. However, the
  implementation of our firmware and the evaluation of it should prove
  sufficient for our thesis. Should the OQRNG-device not be connected to our
  microcontrollers, we'll revise the thesis body to reflect that -- however, the
  main text should not require significant overhauls.
  - One alternate test instead of using the OQRNG-device is to use a separate
    microcontroller that can 'simulate' the OQRNG-device by outputting a stream
    of pseudo-random bits for our MCU to process. This allows us to stress-test
    the system.
- Our iterations are proceeding slightly faster than expected, however, none
  show significant improvements over the first naive implementation. While this
  in and by itself is a result in some aspect and not something we can control
  or impact, it's worth mentioning.
- A lot of the practical work has gone towards developing a suitable test script
  which currently is detailed far too little. We'll consider adding more details
  about this to the main thesis body or to an appendix text.
- Deploying this code to other MCUs has proven challenging, not due to the
  hardware constraints but rather due to the fact that they are significantly
  harder to flash to and work with than Teensy 4.1. We can't adjust much to this
  risk, but need to be aware of it going forward as to not cause undue problems
  down the line.

## Week 14

- [x] Planning seminar 1/4
- [x] Potentially book a meeting with the customer for ADC discussion / order
- [x] Finalize setting up development environment

## Week 15

_NOTE_: At some point before the half time seminar, we should get access to the
ADC and should be able to attempt connecting the ADC to the live equipment and
get real samples. As it's difficult to say exactly when the ADC is delivered and
how much effort it will take to interface with the OQRNG-device, it's hard to
say when we'll work on this. Assume that work before the corresponding deadline
will be added.

- [x] Proceed with first iteration, or the baseline (naive) implementation,
      potentially run it on local hardware first before deployment to MCU
- [x] Generate some first instances of test data (at least one set of test data
      is needed at this point)
- [x] If time allows, deploy naive implementation on MCU and set up time
      measurement code
- [x] If not, test the baseline on local hardware with generated test data to
      verify functionality
- [x] Investigate feasibility of ESP32

## Week 16

_NOTE_: To consider an iteration complete at this point (e.g. when it's to be
finalized) the code should be deployed to Teensy 4.1 and time should be
measured. Experiments should be summarized to easily fit them into the thesis.

- [x] **DEADLINE**: Naive implementation deployed on MCU and time measurement
      set up
- [x] **DEADLINE**: Sets of test data finalized
- [x] Proceed with second iteration (e.g. efficient data structures)
- [x] Create and populate external document with test results (to be filled with
      all coming iterations)
- [x] Investigate feasibility of Pico 2

## Week 17

- [x] **DEADLINE**: Finalize second iteration
- [x] **DEADLINE**: Decide whether ESP32 and Pico 2 are viable candidates for
      comparison (to allow for time to solder parts)
- [x] Proceed with third iteration (e.g. bitshifting)
- [x] **DEADLINE**: Preparation for half time seminar
- [x] Ensure completed experiments are detailed in the thesis, discussion
      started

## Week 18

- [ ] Half time seminar 30/4
- [ ] **DEADLINE**: Finalize third iteration
- [ ] **DEADLINE**: Teensy 4.1 connected to OQRNG-device to sample real data
      (Started 2025-04-25, almost finalized)
  - _NOTE_: This heavily depends on when the ADC is delivered, but should be
    done here at the latest!
  - _NOTE_: Currently, we're holding off on using an external ADC at all,
    possibly going for the internal ADC on our MCU instead.
- Extra time alloted for finishing up trailing tasks

## Week 19

- [ ] Proceed with fourth iteration (e.g. batching) (Preliminarily started
      2024-04-25)
- [ ] **DEADLINE**: Go over potential feedback from half time seminar, adjust
      content accordingly

## Week 20

_NOTE_: At this stage, we should have four iterations completed that
(theoretically) should be MCU agnostic, which we can then deploy to other MCUs
as well. As the last iteration is **not** hardware agnostic, this is the
iteration that might not be deployed to all MCUs as this requires a full rewrite
of all code.

- [ ] **DEADLINE**: Finalize fourth iteration
- [ ] **HARD DEADLINE**: Iterations 1 - 4 finalized!
- [ ] **DEADLINE**: Experiments detailed in the thesis, discussion started
- [ ] Begin work of deploying iterations 1 - 4 on the MCUs that were deemed
      feasible
- [ ] Begin rewrite of code for Teensy 4.1 for the fifth iteration (e.g. ARM
      hardware instructions)
  - [ ] _NOTE_: The ARM instructions, e.g. the fifth iteration, might be too
        time consuming to actually finish. It's hard to know beforehand whether
        this is the case.

## Week 21

- [ ] Begin fifth iteration (e.g. ARM hardware instructions -- _NOTE_: This
      iteration may be too time consuming, and we will have to investigate the
      feasibility of this during the project)
- [ ] Begin preparation for opposition presentation
- [ ] Connect Teensy 4.1 to OQRNG-device and operate on real data

## Week 22

- [ ] **DEADLINE**: Deploy iterations 1 - 4 on weaker MCUs (e.g. ESP, Pico) and
      summarize results in our table
- [ ] **DEADLINE**: Experiments and discussions finished before opposition
- [ ] Opposition presentation 28/5
- [ ] Begin work on opposition feedback as soon as possible after the seminar
- [ ] **DEADLINE**: Fifth iteration finalized

## Week 23

- [ ] **HARD DEADLINE**: All iterations should be finalized on all MCUs during
      this week!
- [ ] **DEADLINE**: Opposition comments should be adjusted in the thesis at the
      end of this week
- [ ] Prepare for final presentation

## Week 24

- [ ] Exact work that needs to be done before the final presentation is a bit
      vague, and will be populated over time as we realize what needs to be
      finished before the final presentation
- [ ] Final presentation 10/6
- [ ] **DEADLINE 13/6**: Finalize report for publishing

## Week 25

- [ ] **HARD DEADLINE 16/6**: Publishing
- [ ] **DEADLINE 17/6**: Reflection document

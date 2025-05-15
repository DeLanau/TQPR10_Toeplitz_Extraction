---
title: "Compiling Markdown to LaTeX"
author:
  - name: "Love Arreborn"
    affiliation: "Linköping University"
    email: "lovar063@student.liu.se"
  - name: "Nadim Lakrouz"
    affiliation: "Linköping University"
    email: "nadla777@student.liu.se"
documentclass: sigchi
bibliography: style/sample.bib
csl: style/ieee.csl
keywords: [Pandoc, LaTeX, CHI Proceedings, Markdown]
abstract: |
  TODO
header-includes:
  - \pagenumbering{arabic}
  - \numberofauthors{2}
---

# 1 INTRODUCTION

In computer science, there are many applications for randomly generated numbers.
From generating keys for cryptography, salting password hashes, load balancing
in distributed systems, memory addressing and so much more. However, the process
of producing these random numbers tends to be pseudo-random, e.g. utilizing the
current states of various modules [@randomness]. These numbers do not generate
true randomness, and in order to heighten security other methods of generating
these are required. Current random number generators (_RNG_) are usually
implemented in code, using certain states of the host machine as a starting
point before running a predetermined algorithm [@randomness]. This pseudo-random
number generation (_PRNG_) comes with the drawback that the result is always
deterministic, provided that the initial state is known.

Imagine, then, if a malicious attacker somehow manages to ascertain the state a
computer was in when it generated a random number, for instance to produce an
SSH-key. This attacker then has the opportunity to accurately reproduce the
exact, deterministic state that produced said random number, in essence removing
the safety that randomness brings. While it may sound unrealistic, the
exponential increase in processing power and the burgeoning field of quantum
computing does introduce the possibility that one day, what we perceive as
random is nothing more than a simple algorithm to crack.

True random numbers, then, cannot be produced solely through code. These systems
require some input that is neither replicable nor reproducible. One method that
can be realistically used is the inherently random movement of lava lamps
[@lavarand], which is used as a backup source of randomness for Cloudflare[^1].
Another proposed solution for this is quantum random number generation (_QRNG_)
[@QRNG]. By reading quantum fluctuation signals from any given source, for
instance an optical signal, the inherent natural unpredictability of said signal
can be harnessed in order to produce a random number from a state that is nigh
impossible to reproduce accurately.

[^1]:
    [Cloudflare.com, accessed 2025-03-10](https://blog.cloudflare.com/randomness-101-lavarand-in-production/)

Further details about how this signal is produced and sampled will be introduced
in section 2 and 3, and builds on the work of Clason [@Clason2023]. This optical
signal needs to be converted to a stream of random, raw bits via an Analog to
Digital Converter (_ADC_). Some processing of the data has to be done in order
to ensure that the data is workable and to remove potential deterministic
patterns from these random bits. One method for this post-processing we will
explore in this work is Toeplitz extraction [@toeplitz], typically performed on
the host computer utilizing the randomly generated numbers. While this system
could very well be implemented on physically larger hardware, thus avoiding the
constraints that limited hardware introduces, Clason proposes a simpler and
cheaper way to achieve QRNG [@Clason2023]. In keeping with this, utilizing
microcontrollers rather than a host computer for processing the raw bits
extracted further helps to keep the costs low and the solution reasonably
complex. Additionally, this allows the system to be self-contained, for instance
a simple and portable USB-device which both generates and processes true random
numbers. Further details regarding the microcontrollers used in our work will be
outlined in section 3.

However, due to the lower processing power of the average microcontroller, any
implementation of Toeplitz extraction needs to work quickly and efficiently
despite this hardware constraint. Experimenting with efficient implementations
of this well-defined algorithm is the main focus of this thesis, and the key
research areas explored in this work are as follows:

**Research area 1 (_RA1_)**: How can Toeplitz extraction be implemented as
effectively as possible on resource constrained hardware in order to process raw
bits into a workable random number?

Toeplitz extraction been optimized quite well, and previous research can be
utilized for this. However, there are still considerations when implementing the
firmware for the microcontroller in order to optimize the code. Our goal is to
attempt several implementations in order to find the most optimal implementation
with the least amount of effective processing time spent on the algorithm.

**Research area 2 (_RA2_)**: Can we ensure that the output of random numbers is
not primarily limited primarily by our implementation, but rather limited by the
processing power or the USB transfer speed of the microcontroller, alternatively
by the ADC?

There will unequivocally be a bottleneck for the processing speed. For instance,
the speed at which the ADC can process the optical signal into raw bits as well
as the speed that the USB output can transfer processed random number to the
host computer will be limiting factors. Further details on the limitations of
the ADC will be outlined in section 3. The slowest of these bottlenecks will
inevitably be the limiting factor for any implementation. Our research aims to
ensure that our implementation of Toeplitz extraction does not become the
limiting factor, but rather processing data fast enough to match or exceed the
speed of the hardware.

Section 2 of this article will introduce the theory that allows for QRNG, and
how this will be utilized in our works. Section 3 delves further into the
hardware and algorithms our work will use, with related works in optimizing
Toeplitz extraction listed under section 4. Section 5 will present our
methodology and implementation strategy, as well as some limitations imposed on
our work. Finally, section 7 will present the results of our experimentation.

## 2 THEORY

A majority of the research around this topic stems from physics, with
implementations of the technology frequently being published and studied by
physicists. As such, a brief introduction to the concepts used in previous
research as well as an introduction to previous implementations of this
technology will be presented in this section.

The idea of an optical QRNG (_OQRNG_) is not a novel one. The basis of the
theory is that intrinsically random properties of a quantum process. Stefanov
et. al. [@StefanovOptical] proposes using the random choice of a photon between
two output signals to generate a random stream of bits, however the theory
behind it can be applied to other quantum processes as well. This particular
theorem has been implemented by Wayne et. al. [@Wayne] to create a quantum
number generator. While this article proves the efficacy of OQRNG, it utilizes a
slightly different method.

### 2.1 Shot noise quantum fluctuations

Our work revolves around the measurement of shot noise of vacuum states rather
than measuring arrival times of photons. Essentially, this is another quantum
process with the same inherently random properties as described by Stefanov et.
al. [@StefanovOptical], but instead using shot noise. As described by Niemczuk
[@shotnoise], shot noise is minor fluctuations in an electrical current, which
is inherently random. Reading this property, then, gives us an intrinsically
random source from which to generate a random output, which in turn can be
processed into a random number.

Implementations of this theory exist, however with significant drawbacks. Shen
et. al. [@contender1] presents an implementation using a fairly complex setup,
in which a continuous-wave fiber laser is the optical source. They conclude that
sampling the shot noise is, indeed, suitable for OQRNG. However, the
implementation requires expensive and complex hardware, and the sheer size of
the system prohibits it from being portable and easily reproducible in
small-scale tests.

A more recent implementation of OQRNG in a smaller scale has been presented by
Singh et. al. [@singh]. This particular implementation uses a bespoke circuit
board where all components are present on a single board -- e.g., this
experimental setup contains an integrated ADC, post-processor, entropy
controller and entropy generator. While this article cements the viability of
OQRNG using shot noise (despite the article not being confirmed as peer
reviewed), the bespoke nature of the circuit board makes this experiment
difficult to reproduce. As our thesis will use commercially available ADCs and
microcontrollers, the only bespoke component is the shot noise generator itself.
Furthermore, the Toeplitz extraction is not run on the microcontroller itself in
these experiments -- instead, the hashing of these raw bits is done on the
receiving computer as this bespoke circuit board featured a relatively weak
processor.

In summary, previous research has proven that OQRNG can generate true
randomness, and more specifically, Shen et. al. [@contender1] and Singh et. al.
[@singh] both implement OQRNG through readings of shot noise. However, there are
limitations in both of these works. Either the system that generates the shot
noise is large and complex [@contender1] or the system is built on bespoke
hardware with limitations in processing power which prevents a fully integrated
system [@singh]. Furthermore, to the best of our knowledge, most of the work in
this field is from the perspective of physicists, and there appears to be little
research on this subject in the domain of computer science. Our work aims to
bridge this gap by using commercially available hardware (other than the bespoke
shot noise generator [@Clason2023]) and focuses on implementing Toeplitz
extraction directly on the microcontroller. Rather than focusing on the
intricacies of quantum fluctuations, we will instead approach this problem from
a computer science perspective.

## 3 BACKGROUND

Our work is a practical continuation of the work of Clason [@Clason2023]. In
this work, quantum shot noise originating from photodiodes was studies, and in
so doing a prototype device was constructed. This device read from an optical
source, outputting analog voltage from the data "seen" by the diodes. A
prototype was constructed, in which an LED is read by a photodiode soldered
millimeters apart. In this section, we introduce the hardware used for our
implementations as well as the considerations taken in order to shift the focus
from physics to computer science.

### 3.1 Optical RNG module

The one bespoke piece of hardware used in this study is the prototype designed
by Clason [@Clason2023] as a part of his masters thesis. This device produces
the optical shot noise which will be the source of randomness in our work.
Moving forward in this article, we will refer to this as the OQRNG-device.

As described in Clasons work [@Clason2023], the OQRNG-device is an
electro-optical system which measures optical shot noise, generating quantum
randomness. The device has an LED and a photodiode positioned a few millimeters
apart, ensuring efficient light coupling. The photodiode detects light from the
LED, and converts the light into a current signal, which is sent to a
transimpedance amplifier to convert it into measurable voltage. In order to
minimize disruptions by other external lights, the system is enclosed in a
shielded measurment box.

Whereas the exact quantum mechanisms that ensure that this system ensures
randomness and further details regarding the OQRNG-device is better derived
directly from Clasons work [@Clason2023], the end result as it correlates to our
study is an inherently random, analog voltage current.

### 3.2 ADC converter

This analog current isn't suitable to operate on without further processing. As
mentioned in section 1, the signal needs to pass through an ADC and converted
into raw bits in order for it to be usable. In his thesis, Clason [@Clason2023]
suggests a discrete ADC chip capable of analyzing frequencies higher than 25
MHz, as this is the highest frequency studied in his work. However, in the
interest of keeping the implementation light and cheap, we will be using ADCs
that provide less samples per second and lower frequencies. This is done mainly
for ease of development and access to this hardware.

Many microcontrollers furthermore come equipped with internal ADCs that can be
utilized, and while these provide a lower sample size (often around 1 MSPS), the
ease of development may be prudent to utilize for this proof-of-concept. While
our initial ADC has a fairly low throughput, this can always be upgraded if it
ends up becoming too limiting.

Should these internal ADCs prove too limiting, we propose utilizing
MAX11102AUB[^3] with an effective sample rate of 2 million samples per second
(MSPS). This ADC provides a 12 bit sample size, providing roughly 24 Mbit/s of
sampled data per second, derived by the following calculation.

$$
\text{ADC Throughput}
= \frac{2,000,000_{\mathrm{MSPS}}\times12}{1,000,000}
\approx24\ \mathrm{Mbit/s}\phantom{123}(1)
$$

The final output from the ADC, whether built into the microcontroller or an
external one, will be a stream of raw bits, as the analog signal from the
OQRNG-device is processed.

### 3.3 Microcontroller

Microcontrollers (MCUs) are compact and low-power computing devices designed for
embedded systems and real-time operations, and suitable as a processing unit for
the purposes of this work. Unlike general purpose CPUs, an MCU integrates a
processor, memory and peripherals (_such as an ADC, for instance_) into a single
chip. Furthermore, modern MCUs often feature advanced microarchitectural
elements to enhance processing capabilities on single threads (_such as
dual-issue superscalar architectures, allowing the MCU to run several
instructions per CPU cycle_), making them suitable candidates for the
post-processing required for OQRNG-data.

<!--
Microcontrollers (MCUs) are compact and low-power computing devices designed
primary for embedded systems and real-time applications. Unlike general purpose
CPUs, MCUs integrates a processor, memory, and peripherals into a single chip.
Thus, enabling efficient and autonomous operation in constrained environments.
Various development frameworks and tools are available for programming MCUs,
including the Arduino Framework, manufacturer specific SDKs and raw C++, among
others. Each framework presents different trade-offs in terms of performance
overhead, efficiency, and ease of development.

Modern microcontrollers feature advanced microarchitectural elements to enhance
processing capabilities. Some of high performance MCUs utilize dual-issue
superscalar architectures, enabling them to run several instructions in one
cycle. This type of design greatly improves single-threaded capabilities, which
makes these MCUs ideal for tasks that need real-time data processing. The use of
dual-issue architectures in embedded devices has demonstrated high execution
speeds, along with the added benefit of improved energy efficiency. Thus, making
them a great option for applications that require low power but high
performance. Research conducted by Lozano et al. [@dual-issue] specifically
explores energy efficient dual-issue processors designs. This study shows that
optimizing instruction throughput and power consumption can significantly
improve processing efficiency in embedded systems. While energy efficiency is
not the primary goal for this study, ensuring that the resulting USB-device does
not consume too much power is tangentially relevant for real-world usage.

-->

Since MCUs often function under strict timing requirements, it is critical to
have effective ways to access memory and transfer data for processing in real
time. High-performance MCUs enhance memory usage in various ways. Some of them
use Tightly Coupled Memory[^3] (TCM), which gives fast SRAM with specific access
routes for important data, avoiding cache misses and guaranteeing consistent
performance. Moreover, instruction and data caching techniques, including
instruction prefetching and branch prediction, help minimize execution delays in
computationally intensive real-time applications. Another important aspect is
Direct Memory Access[^5] (DMA), which enables data transfer between peripherals
such as the ADC and RAM, without CPU intervention. This offloading reduces
processing overhead, allowing the MCU to manage fast data transfers effectively.
These improvements are especially significant for Toeplitz extraction, where
large amount of random data needs to be processed and sent quickly with low
delay. Efficient memory management guarantees that randomness extraction can
occur rapidly without major slowdowns in computing. Both approaches will be
tested during development.

[^3]:
    [Arm developer documentation, accessed 2025-03-13](https://developer.arm.com/documentation/den0042/a/Tightly-Coupled-Memory)

[^5]:
    [ScienceDirect Journals & Books, accessed 2025-03-13](https://www.sciencedirect.com/topics/computer-science/direct-memory-access)

In our work, we intend to use Teensy 4.1[^2], based on the ARM Cortex-M7. This
MCU is especially suitable for computationally demanding tasks involving
randomness extraction due to its dual-issue superscalar architecture and Digital
Signal Processing (_DSP_) capabilities. The floating-point unit[^4] (FPU) and
Single Instruction, Multiple Data (_SIMD_) style DSP instructions improves how
quickly it can perform bitwise and arithmetic tasks, which are crucial for quick
Toeplitz extraction. SIMD-controlled DSP architectures, as described by Han et
al. [@simd-dsp], leverage parallel vectorized computation to accelerate matrix
operations -- making them highly effective for Toeplitz matrix-vector
multiplications.

[^3]:
    [Technical specification for MAX11102AUB, accessed 2025-03-13](https://www.farnell.com/datasheets/1913106.pdf)

[^2]:
    [Teensy developer documentation, accessed 2025-02-27](https://www.pjrc.com/store/teensy41.html)

[^4]:
    [ScienceDirect Journals & Books, accessed 2025-03-13](https://www.sciencedirect.com/topics/computer-science/floating-point-unit)

In order to evaluate how efficent our implementation can become, our aim is to
try our implementation on other MCUs with varying levels of power and hardware
support. Whereas Teensy 4.1 is our primary development platform which we will
evaluate closely, we aim to run our implementations on Raspberry Pi Pico 2[^6]
as well as ESP32-S3[^7]. Due to the lower computational power of these MCUs,
there may be significant issues in utilizing these weaker models, yet they are
significantly cheaper and easier to access. Testing of these will consist solely
of running the implementation on these controllers and measuring execution speed
and correctness of the output.

[^6]:
    [Raspberry Pi Pico 2 documentation, accessed 2025-03-13](https://datasheets.raspberrypi.com/pico/pico-2-product-brief.pdf)

[^7]:
    [ESP32-S3 documentation, accessed 2025-03-13](https://www.espressif.com/sites/default/files/documentation/esp32-s3_datasheet_en.pdf)

### 3.4 Toeplitz extraction

The raw bits from the ADC can potentially have some deterministic patterns, and
as such have to be processed somehow in order to remove these patterns. Several
methods exist for this purpose, and for our work, we will perform this
preprocessing via Toeplitz extraction. The main focus of this study is to
implement this extraction algorithm as effectively as possible on resource
constrained hardware.

A detailed account of the inner workings of Toeplitz extraction can be found in
the work of Chouhan et al. [@toeplitz-desc]. This work focuses on implementing
Toeplitz extraction on field-programmable gate-arrays (FPGA), but some specific
details can be derived from their work. As these authors describe, Toeplitz
extraction is a strong contender for our work due to a lower computational
complexity than other alternatives, as well as a relatively easy algorithm to
use. This extraction utilizes either matrix multiplication or hashing between a
pseudo-random seed and the raw data provided from a high-entropy source of
randomness -- in our case, the OQRNG-device.

To summarize the theoretical working of Toeplitz extraction (as explained by
Chouhan et al. [@toeplitz-desc]), a pre-determined seed matrix ($T$) is
multiplied with the sampled raw bit matrix ($K$). The size of the seed is
directly dependent on the size of the sampled data, and can be fixed or
continually re-sampled as needed. To ensure high levels of entropy, our
intuition is that re-sampling the seed from the OQRNG-device continually is
prudent. The sample and seed with then be processed with matrix multiplication
to remove deterministic patterns, and produce a bitstring that results in our
randomly generated number. An example of how this extraction works can be seen
below.

```
Where n = number of input bits,
      m = number of output bits:

Let X be the input array (length n)
Let T be the seed matrix (length n + m – 1)
Let Y be a new array of length m

For i from 0 to m - 1:
  sum = 0
  For j from 0 to n - 1:
    sum = sum + X[j] * T[i + j]
  Y[i] = sum mod 2

Return Y
```

The main focus of this work is implementing this algorithm as efficiently as
possible on our MCUs, and as such, several optimization efforts need to be taken
into account during our experimentation.

### 3.5 Summary

With the assumption that the OQRNG-device produces a truly random analog signal,
we can now clearly define the scope in which this thesis operates. Considering
the maximum conversion speed from the ADC and the USB-output from the MCU, we
have a clear bound over 24 Mbit/s (imposed by the ADC) in which Toeplitz
extraction needs to be executed. Any speeds over 2.86 MB/s allows us to upgrade
the ADC iteratively to continue increasing the output speed. Any implementation
of Toeplitz extraction must then execute fast enough on any given
microcontroller feasible for the proposed quantum RNG-thumbstick as to not be
the decisive limiting factor.

## 4 RELATED WORKS

Most related works found tend to revolve around optimizing Toeplitz using more
advanced hardware, and few works seem to delve into evaluating the
implementation in code.

As mentioned in section 3.4, Chouhan et al. [@toeplitz-desc] utilized FPGA to
implement Toeplitz extraction specifically for OQRNG, a work also studied by
Zhang et al. [@toeplitz]. Furthermore, Zhang et al. utilized a standardized
min-entropy evaluation to ensure true, unbiased randomness in their result. Both
of these implementations utilize powerful hardware where the matrix
multiplication is offloaded to FPGA. These implementations provided extraction
speeds of between 3.36 Gpbs [@toeplitz] to 26.57 Gbps [@toeplitz-desc]. Neither
of these implementations operate on constrained hardware, instead creating
bespoke circuit boards for their works.

Another important point that is often neglected in randomness extraction using
Toeplitz matrices is how the seed key is handled when forming the Toeplitz
matrix. Numerous systems, such as those developed by Chouhan et al.
[@toeplitz-desc] and Zhang et al. [@toeplitz], depends on fixed seeds. Fixed
seeds can create security risks over prolonged operation time. To tackle this
problem, Lin et al [@lin] proposed a method for seed-renewable Toeplitz
post-processing in QRNG. Their strategy incorporates a dynamic seed pool within
the FPGA, where each instance of post-processing picks a new, randomly selected
seed. Thus, minimizing temporal correlations between extractions. Furthermore,
an external seed updating mechanism via PCIe ensures that seeds are refreshed
whenever a certain security limit is reached. Compared to fixed-seed methods,
this renewable approach enhances cryptographic robustness and ensures sustained
high-security randomness extraction in real-world applications.

Efficient Toeplitz matrix-vector multiplication (TMVM) is critical for
optimizing randomness extraction that relies on Toeplitz, particulary in
constrained hardware environments. Liao et al. [@liao] showed that this process
could be greatly accelerated using Fast Fourier Transform (FFT) and its inverse
(IFFT). Thus, reducing computational complexity from $O(n^2)$ to $O(n log n)$.
Their implementation on FPGA utilized this approach for deep neural networks,
resulting in a 28.7 times decrease in model size while still achieving fast
inference speeds. By using FFT and IFFT acceleration, Toeplitz post-processing
for randomness extraction could achieve higher throughput. Thus, potentially
could improve performance and reduce latency.

## 5 METHODOLOGY

With the consideration that our work revolves around optimizing Toeplitz
extraction in order to quickly process random bits into a random number, we will
take an iterative approach. For our tests, we will use a pre-defined stream of
raw bits which is sent to the microcontroller via USB, and run several different
implementations of Toeplitz extraction to produce numbers. As we always use a
pre-defined bitstream, the result will at this stage be deterministic, giving us
a clear indication whether the algorithm works as intended.

However, in order to ensure the results work with varying data, we cannot limit
ourselves to simply one stream of bits. The main point of the algorithm is to
remove patterns in the bitstream that may lead to less randomized results. As
such, we will sample several bitstreams from the OQRNG-device to use for our
tests -- each with varying degrees of repeated patterns that should be
eliminated by the algorithm. All bitstrings tested are available as an appendix
to this paper.

<!-- TODO: add figure for testing setup -->
<!-- TODO: move some details from experementation sectio to implementation strategy and remove whole experemintation section, as well as some part of results. e.g concentrate every important thingy about implementation and testing setup here in section 5. -->

### 5.1 Implementation strategy

Our implementation of the Toeplitz extractor followed a structured, iterative
approach divided into two phases. Phase one focused on exploring performance
improvements through incremental algorithmic changes. Phase two then addressed
architectural inefficiencies identified during phase one. Each phase then
consisted of a number of individual iterations, in which performance was
evaluated in terms of both correctness and execution time on the target
hardware.

#### 5.2 Phase one

Initially, we require a "naive" version designed to prioritize correctness over
speed. This version was first executed on a separate computer to generate
reference output for various input bitstrings, which were later used as accuracy
baselines. The naive implementation was then flashed onto the Teensy 4.1
microcontroller, where execution time was measured in microseconds. Each
subsequent iteration introduced controlled modifications aimed at improving
throughput.

The initial implementation followed the pseudocode described in section 3.4,
using matrix multiplication over raw input and seed data. It relied on
`std::vector<int>` for storage and used nested loops to compute each output bit.
From this point, this implementation was improved over the coming iterations,
and the new implementation verified in the same manner as the initial version.

**Iteration 1 - Data structures:** This iteration kept the same algorithmic
logic as the initial implementation, but experimented with data structures such
as raw pointers and hash maps to enhance the use of `std::vector`. The goal was
to reduce or eliminate the reliance on nested loops.

**Iteration 2 - Bitshifting:** Basic bitwise operation was introduced to replace
arithmetic whenever possible. Multiplication was replaced with logical `AND &`
and modulo operations with bit masking `& 1`. The goal was to reduce the number
of instructions and improve per-bit processing speed.

**Iteration 3 - Batching and Hardware optimization:** This iteration focused on
optimizing performance through batching and the use of ARM-native instructions,
beginning by testing batching alone. This was followed by isolated use of ARM
instructions such as `__builtin_popcountll()` (_which counts the number of set
bits in an unsigned integer_). After establishing their individual effect we
combined both techniques, multiple batch sizes were tested to determine their
impact. More details and benchmarks for each configuration can be found in the
section 6.

#### 5.3 Phase two

Phase two focused on addressing inefficiencies and design issues that were
unintentionally introduced during earlier iterations. Rather than continuing
with new algorithmic ideas, this phase aimed to identify and fix structural
problems. Several assumptions from phase one were re-evaluated -- such as the
benefits of certain data structures or abstractions.

**Iteration 4 - Loop unrolling:** This iteration focused on reducing the number
of loops in the extractor by manually unrolling repeated operations. The goal
was to decrease overhead created by loops. Whereas this operation is commonly
done by compiler optimization, manually performing this guarantees that we
unroll the loops rather than leaving it to the compiler.

**Iteration 5 - Removal of vector usage:** This iteration removed `std::vector`
in favor of fixed-size types like `uint32_t` and `uint64_t` to reduce the
overhead introduced by creating and populating this complex data structure.

**Iteration 6 - Data type exploration:** Following the removal of vectors, this
iteration explored alternative static data types to determine the most efficient
structure for storing input and seed data.

**Iteration 7 - Re-implementation of bitshifting:** In this final iteration, bit
shifting techniques were reintroduced into the now simplified implementation.
With high-level abstractions removed and memory structures streamlined,
bit-level operations such as shifting and masking were applied.

### 5.4 Evaluation

To evaluate the correctness of each implementation, a baseline was generated as
discussed in section 5.2. Using the naive, initial implementation to process
bits and saving for later evaluation gave us a source of truth against which to
compare following iterations. To verify that the algorithm successfully removed
the patterns it should, we verified the measured entropy score with the command
line utility `ent`[^8].

[^8]:

[Manual page for `ent`, accessed 2025-04-23.](https://manpages.ubuntu.com/manpages/trusty/man1/ent.1.html)

We created a script to facilitate easier testing, which is attached to this
paper. Using this script, all bitstrings used for testing can be evaluated
against the baseline, ensuring that the output from the new iteration matches
the baseline exactly. Furthermore, the script also provides the average
execution time of only the Toeplitz extraction in microseconds.

Hyncica et. al. [@micromeasurements] propose that measuring execution time of
algorithms directly via the microcontrollers internal timers (while subtracting
the interrupt overhead) provides adequate measurements of an algorithms
execution speed. An additional advantage is that the same code can be used to
measure execution speed on several different microcontrollers, rather than
relying on counting CPU cycles (as the process for this may vary greatly between
controllers). As we will use fixed-size bitstrings for evaluation, we can then
derive the throughput of the algorithm in $Mbit/s$ as follows:

$$
\text{Throughput}{\mathrm{Mbit/s}}
= \frac{DataSize{\mathrm{bits}}}{ExecutionTime_{\mathrm{ms}}}
\times 10^{-3}
\phantom{12}(2)
$$

This measurement allows us to place the throughput of our algorithm soundly in
the bounds imposed on us by the hardware. Plugging in the $24 Mbit/s$ bound
imposed by the ADC with an arbitrarily chosen 64-bit sample size, we can derive
the average execution speed in microseconds:

$$
T_{\mathrm{ms}}
= \frac{64}{24}\times10^{-3}\ \mathrm{ms}
\approx 2.667\times10^{-3}\ \mathrm{ms}
=2.667\ \mu\mathrm{s}.
\phantom{12}(3)
$$

In section 6, this calculation will be used to derive the execution speed of the
various iterations.

### 5.3 Limitations

Our proposed iterations all assume that the limited hardware will support it.
Whereas we are confident that Teensy 4.1 will be able to handle each iteration
step (even the naive implementation), the remaining microcontrollers with lower
specifications might not be suitable for the first iterations. Testing the
implementations on different microcontrollers could turn out to be unfeasible --
however, this remains to be seen during the experimentation.

## 6 RESULTS

<!-- NOTE: IT'S TABLE TIME -->

\begin{tabular}{|c|c|c|c|c|c|c|} \hline \textbf{Bit size} &
\multicolumn{3}{c|}{\textbf{Teensy ($\mu s$)}} &
\multicolumn{3}{c|}{\textbf{Pico ($\mu s$)}} \\ \hline & Min & Max & Avg & Min &
Max & Avg \\ \hline 64 & 13 & 14 & 13.1564 & 103 & 117 & 106.3914 \\ 512 & 788 &
791 & 788.3139 & 5296 & 5311 & 5302.4979 \\ 1024 & 3124 & 3130 & 3124.0580 &
21108 & 21119 & 21111.2163 \\ \hline \end{tabular} \caption{Minimum, maximum and
average processing time for Teensy and Pico.}
\label{tab:teensy_pico_min_max_avg}

Table 1 presents the execution speeds of each iteration as listed in sections
5.3 and 5.4, alongside the exectution speed in Mbit/s as calculated by utilizing
the average value in (3). My thought then is that we here talk about the results
and what they demonstrate, before moving on to draw conclusions and attempt to
answer our research questions in section 7.

<!--

##7 CONCLUSION mb first present raw results, and than explain them/draw conclusion ? idk

All iterations was tested on 6 different input datasets, with some results
varying less than 0.1 microseconds. These results with such slight variations
have been omitted from the table. All the tables shown below has been tested on
the dataset `biased_bits_bad.bin` which can be found as an appendix -- and
whereas more files have been used, this file has been arbitrarily selected to
present the data.

Due to compatibility issues, Pi Pico 2 was tested on different machine, creating
further variation in the results -- about 1 microsecond.

### Iteration 1

The first iteration brought few surprises, as it ran quite well on Teensy. To
our surprise, it executed well enough on Pi Pico 2 as well, albeit with
\~$8\times$ slower execution speed. This first implementation is the same one
used to generate the baseline results, and used to ensure following iterations
all generate correct output.

Worth noting is that this iteration (_and all following iterations that generate
the same result_) have achieved entropy scores from `ent` that are greater than
7.99 for all tested data while ensuring it cannot be compressed further. This
ensures that the algorithm has successfully removed as much of the patterns as
can be reasonably expected from the raw binary data while maintaining the random
nature of the data.

\begin{tabular}{|c|c|c|c|c|c|c|} \hline \textbf{Bit size} &
\multicolumn{3}{c|}{\textbf{Teensy ($\mu s$)}} &
\multicolumn{3}{c|}{\textbf{Pico ($\mu s$)}} \\ \hline & Min & Max & Avg & Min &
Max & Avg \\ \hline 64 & 13 & 14 & 13.1564 & 103 & 117 & 106.3914 \\ 512 & 788 &
791 & 788.3139 & 5296 & 5311 & 5302.4979 \\ 1024 & 3124 & 3130 & 3124.0580 &
21108 & 21119 & 21111.2163 \\ \hline \end{tabular} \caption{Minimum, maximum and
average processing time for Teensy and Pico.}
\label{tab:teensy_pico_min_max_avg}

### Iteration 2

Although the updated version included enhanced data structures, theoretical and
practical analysis indicated that these modifications do not provide advantages
for this project. Due to the small size of the input data (64, 512, 1024 bits) -
using plain, contiguous memory layouts such as `std::vector<int>` proves to be
very efficient in terms of access speed and memory usage. Current C++ compilers
can effectively optimize these simple structures, often generating faster and
more compact machine code than manually optimised alternatives. Introducing
additional data structures or indirection would only add overhead without
improving performance.

From an algorithmic perspective, Toeplitz extraction inherently requires a
double summation loop, which leads to a time complexity of $O(n^2)$ no matter
the optimization applied internally. Thus, making attempts to change it would
not result in lower asymptotic cost.

In summary, given the size and type of this system, a straightforward and
minimal approach is not just adequate, but also recommended.

### Iteration 3

Coming soon!-->

## CHANGELOG

2025-02-14: Added background section, smaller reviews to introduction.

2025-02-28: Template adjusted, added methodology. Started review of background
and theory to add stronger correlation to computer science. Not yet finished due
to review of articles as well as some additional information required from the
project owner. The update to theory and background should be considered a heavy
work in progress at this stage.

2025-03-10: Moved evaluation down in the methodology in order to provide a
better flow. Elaborated further on Toeplitz extraction and ADC converters, as
well as motivating the selection of these. Some additional information added in
introduction as motivation for the work.

2025-03-13: Elaborated on background, as well as adding more details regarding
hardware. Note that the hardware selected is subject to change over time.
Further elaborated on related works in optimizing Toeplitz extraction.

2025-04-22: Begun including details regarding initial experimentation, updating
details regarding experiments that had to change (e.g. no baseline on separate
hardware).

2025-04-27: Added first table with test data.

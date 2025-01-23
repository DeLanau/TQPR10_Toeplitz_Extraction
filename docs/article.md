---
title:
  "Creating High-Speed Firmware for Random Number Generating Microcontrollers"
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
  In computer security as well as cryptography, one key aspect revolves around generating truly random numbers. Traditionally, this has been done by utilizing non-predictable modules from the computer itself, such as the system time or similar. This is far from true randomness, and this thesis project is aimed at constructing one part of a random number generator which provides randomness by reading from an optical signal. The natural jitter in this signal will provide more randomness than any internal component can offer. This optical signal is then streamed through an Analog to Digital Converter (_ADC_), which then needs to be processed by a microcontroller. This thesis project aims to create firmware to then process this data into random numbers, with emphasis on creating high-speed implementations of said firmware in order to find the optimal solution for the end product.

header-includes:
  - \pagenumbering{arabic}
---

# 1. INTRODUCTION

T

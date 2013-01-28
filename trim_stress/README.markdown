This directory performs stress testing of "apitrace trim".

For each trace file in this directory the trim_stress.py test driver
will perform the following operations:

Given <program>.trace:

    1. Generate snapshots of original trace in ./<program>-ref/

    2. For each frame of the trace:

        1. Use "apitrace trim" to trim <program>.trace to a
           single-frame trace in <program>-trim.trace

	2. Generate a snapshot of the trimmed trace to
	   ./<program>-out/<frame>.png

    3. Use "apitrace diff-images" to compare all snapshots in
       ./<program>-out with those in ./<program>-ref

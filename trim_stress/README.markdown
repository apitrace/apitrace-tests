This directory performs stress testing of "apitrace trim".

For each trace file in this directory the trim_stress.py test driver
will perform the following operations:

Given <program>.trace:

    1. Generate snapshots of original trace in ./<program>-ref/, (this
       step is skipped if the <program>-ref directory already exists.
       Simply delete this directory before "make test" to force new
       generation of reference images.

    2. For each frame of the trace:

        1. Use "apitrace trim" to trim <program>.trace to a
           single-frame trace in <program>-trim.trace

	2. Generate a snapshot of the trimmed frame to
	   ./<program>-tmp-out/<frame>.png

        3. Use "apitrace diff-images" to compare this one frame with
           the corresponding reference image in ./<program>-ref.  If
           the images differ the entire process is interrupted here.

        4. Move the frame snapshot from ./<program>-tmp-out to
           ./<program>-out

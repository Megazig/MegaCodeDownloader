.globl stuff
stuff:
li		%r4,	250
li		%r5,	80
mullw	%r4,	%r4,	%r5
li		%r5,	2
mullw	%r3,	%r4,	%r5
blr

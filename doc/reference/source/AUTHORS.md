# AUTHORS

## ABOUT 
The Haystack Observatory Postprocessing system (HOPS) has a long lineage
and history of revision and redesign. Initially it was born from the efforts of 
Alan Rogers in the late 70's with a program called FRNGE which was 
written in Fortran and designed to be efficient on an HP-21MX 
(later renamed HP-1000) minicomputer. As hardware and software improvements 
proliferated, a rewrite of the Fortran toolset was launched in
the early 90's by Colin Lonsdale, Roger Cappallo and Cris Niell as
driven by the needs of the geodetic community. The basic algorithms
were adopted from FRNGE; but there was a complete rewrite of the code
into (K\&R) C and substantial revisions of the i/o, control and file
structures resulting in the framework of the HOPS (<=3) system.
This was followed by a substantial effort in the early-mid
2000's to develop tools for optimizing SNR and to derive correction factors
for data with imperfect coherence. While there is no definitive, published, 
HOPS reference in the literature, the Mark 4 Correlator paper ([Whitney](https://doi.org/10.1029/2002RS002820))
touches upon the basic implementation available at that time period.
Further evolution in the late 2000's was provoked by the emergence of software correlation
(DiFX [Deller](https://doi.org/10.1086/513572)), and in the 2010's by the
needs of EHT-scale mm-VLBI. HOPS4 is a continuation/offshoot of the HOPS3 software and 
is a result of a re-architecture/redesign effort undertaken to make the software
more modular and extensible in the face of ever increasing VLBI bandwidths and calibration complexity
and is a work in progress.


## Current Developers/Maintainers

- John Barrett (jpb)
- Geoff Crew (gbc)
- Dan Hoak

## Past Developers/Maintainers

- Roger Cappallo (rjc) - supported the 1990's rewrite and continued development until 2019
- Colin Lonsdale (cjl) - architected the rewrite in the 1990's
- Cris Niell (cmn) - supported rewrite from Fortran to K&R C
- Alan Rogers (aeer) - started it all with FRNGE in the late 1970's
- Kevin Dudevoir (kad) - supported the software late 2000s to 2015

## Other Contributors

- Alan Whitney
- Arthur Niell (aen)
- Brian Corey (bec)
- Dan Smythe 
- John Ball (jab)
- Sheperd Doeleman 
- Chester Ruszczyk
- Vincent Fish
- Lynn Matthews
- Pedro Elosegui
- Lindy Blackburn
- Violet Pfeiffer


> This list is non-exhaustive and reflects major contributions. 

#!/usr/bin/perl

$n = <STDIN> ;
chomp($n) ;
$n =~ s/\[.*\]//g ;

@q = split / /, $n ;
print STDOUT "memset( $q[1], 0, sizeof($q[1]) )\n" ;

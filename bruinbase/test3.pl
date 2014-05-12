#!/usr/bin/perl
use strict;
my $pl = 0;
my @array;
my $i;
for($i=0; $i<100; $i++){
    $array[$i]=$i;
}

for($i=99; $i>0; $i--){
    my $rand =int(rand($i+1));
    my $tmp = $array[$i];
    $array[$i] = $array[$rand];
    $array[$rand] = $tmp;
    print "$array[$i],\"test3\"\n"
}
print "$array[0],\"test3\""

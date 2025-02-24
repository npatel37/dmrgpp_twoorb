#!/usr/bin/perl

use strict;
use warnings;

package Ci;

sub getTests
{
	my ($desc) = @_;
	my @tests;
	my $counter = 0;
	open(FILE,"$desc") or die "$0: Cannot open $desc : $!\n";

	my $prevNumber;
	my $description = "";
	my $barrier = 0;
	while (<FILE>) {
		next if (/^\#/);
		my $line = $_;
		chomp($line);
		$line =~ s/[ \t]+//g;
		$barrier = 1 if ($line eq "");
		if (/^(\d+)\)(.*$)/) {
			my $number = $1;
			if ($counter == 0) {
				$prevNumber = $number;
				++$counter;
				$description = $2;
				$description .= "\n" if ($description ne "");
				next;
			}

			my %h;
			$h{"number"} = $prevNumber;
			$h{"description"} = $description;
			$description = $2;
			$description .= "\n" if ($description ne "");
			$tests[$counter - 1] = \%h;
			$prevNumber = $number;
			++$counter;
			$barrier = 0;
		} else {
			$description .= $_ unless($barrier);
		}
	}

	close(FILE);

	my %h;
	$h{"number"} = $prevNumber;
	$h{"description"} = $description; 
	$tests[$counter - 1] = \%h;

	return @tests;
}

sub isSu2
{
	my ($file,$n) = @_; 
	open(FILE, "$file") or return 0;
	my $su2 = 0;
	while (<FILE>) {
		chomp;
		if (/UseSu2Symmetry=1/) {
			$su2 = 1;
			last;
		}
	}

	close(FILE);
	return $su2;
}

sub procRanges
{
	my ($range, $total) = @_;
	my @inRange;
	return @inRange if (!defined($range));

	my @temp = split(/,/, $range);
	my $n = scalar(@temp);
	for (my $i = 0; $i < $n; ++$i) {
		procRange(\@inRange, $temp[$i], $total);
	}

	my @unique = do { my %seen; grep { !$seen{$_}++ } @inRange };
	return @unique;
}

sub procRange
{
	my ($ranges, $range, $total) = @_;
	my @temp = split(/\-/, $range);
	my $n = scalar(@temp);
	die "$0: FATAL: Empty range $range\n" if ($n == 0);
	if ($n == 1 and $range =~ /^[0-9]+$/) {
		push @$ranges, $range;
		return;
	}

	if ($n == 2) {
		my $start = $temp[0];
		my $end = $temp[1];
		$start = 0 unless ($start =~ /^[0-9]+$/);
		$end = $total unless ($end =~ /^[0-9]+$/);
		for (my $i = $start; $i <= $end; ++$i) {
			push @$ranges, $i;
		}

		return;
	}

	die "$0: FATAL: Invalid range spec.: $range\n";
}

sub getAllowedTests
{
	my ($a) = @_;
	my %hh;
	my $n = scalar(@$a);
	for (my $i = 0; $i < $n; ++$i) {
		my $h = $a->[$i];
		my $n = $h->{"number"};
		$hh{"$n"} = $h->{"description"};
	}

	return %hh;
}

sub helpFor
{
	my ($label) = @_;
	my $h = "";
	if ($label eq "-n") {
		$h .= "\t-n n\n";
		$h .= "\t\tSupply tests to run, this is mandatory.";
		$h .= "\t\tThis is a comma-separated list of at least one range.\n";
		$h .= "\t\tA range is one of the following.\n";
		$h .= "\t\t\tA number, like 2\n";
		$h .= "\t\t\tA number followed by a dash, like 2-; this sets the minimum\n";
		$h .= "\t\t\tA dash followed by a number, like -2; this sets the maximum\n";
		$h .= "\t\t\tTwo numbers separated by a dash, like 2-4, indicating the range {2, 3, 4}\n";
		return $h;
	} elsif ($label eq "-w") {
		$h .= "\t-w workdir\n";
		$h .= "\t\tUse workdir as working directory not the default of tests/\n";
		return $h;
	} elsif ($label eq "-nosu2") {
		$h .= "\t-nosu2\n";
		$h .= "\t\tDo not postprocess SU(2) tests\n";
		return $h;
	} elsif ($label eq "-h") {
		$h .= "\t-h\n";
		$h .= "\t\tPrint this help and exit\n";
		return $h;
	}

	die "$0: No printHelpFor $label\n";
}

sub compactList
{
	my ($str) = @_;
	my @temp = split(/ /, $str);
	my $n = scalar(@temp);
	return $str if ($n < 3);
	my $prev = $temp[0];
	my $begin = $prev;
	my $text = "";
	for (my $i = 1; $i < $n; ++$i) {
		if ($temp[$i] == $prev + 1) {
			$prev = $temp[$i];
			next;
		}

		$text .= ($begin == $prev) ? "$begin, " : "${begin}-${prev}, ";
		$prev = $begin = $temp[$i];
	}

	$text .= ($begin == $prev) ? $begin : "${begin}-${prev}";
	return $text;
}

sub getCiAnnotations
{
	my ($file,$n) = @_;
	open(FILE, "$file") or return "";
	my @whatObserve;
	my @whatDmrg;
	my @whatTimeObsInSitu;
	my @brakets;
	my @metts;
	my $counter = 0;
	while (<FILE>) {
		chomp;
		if (/^\#ci observe (.*$)/) {
			push (@whatObserve, "$1");
			next;
		}

		if (/^#ci dmrg (.*$)/) {
			push (@whatDmrg, "$1");
			next;
		}

		if (/^#ci getTimeObservablesInSitu (.*$)/) {
			push (@whatTimeObsInSitu, "$1");
			next;
		}

		if (/^#ci CollectBrakets (.*$)/) {
			push (@brakets, "$1");
			next;
		}

		if (/^#ci metts (.*)/) {
			push (@metts, "$1");
		}
	}

	close(FILE);
	my %h;
	$h{"dmrg"} = \@whatDmrg;
	$h{"observe"} = \@whatObserve;
	$h{"getTimeObservablesInSitu"} = \@whatTimeObsInSitu;
	$h{"CollectBrakets"} = \@brakets;
	$h{"metts"} = \@metts;
	return %h;
}

1;


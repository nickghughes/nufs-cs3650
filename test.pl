#!/usr/bin/perl
use 5.16.0;
use warnings FATAL => 'all';

use Test::Simple tests => 28;
use IO::Handle;

sub mount {
    system("(make mount 2>&1) >> test.log &");
    sleep 1;
}

sub unmount {
    system("(make unmount 2>&1) >> test.log");
}

sub write_text {
    my ($name, $data) = @_;
    open my $fh, ">", "mnt/$name" or return;
    $fh->say($data);
    close $fh;
}

sub read_text {
    my ($name) = @_;
    open my $fh, "<", "mnt/$name" or return "";
    local $/ = undef;
    my $data = <$fh> || "";
    close $fh;
    $data =~ s/\s*$//;
    return $data;
}

sub read_text_slice {
    my ($name, $count, $offset) = @_;
    open my $fh, "<", "mnt/$name" or return "";
    my $data;
    seek $fh, $offset, 0;
    read $fh, $data, $count;
    close $fh;
    return $data;
}

system("rm -f data.nufs test.log");

my $huge0 = "=This string is fourty characters long.=" x 1000;
write_text("40k.txt", $huge0);
my $huge1 = read_text("40k.txt");
ok($huge0 eq $huge1, "Read back 40k correctly.");

my $huge2 = read_text_slice("40k.txt", 10, 8050);
my $right = "ng is four";
ok($huge2 eq $right, "Read with offset & length");


use v5.14;
use strict;
use warnings;

my @prime = map $_, 0..4800;
@prime = grep { 2 <= $_; } @prime;

my $num = $prime[0];
while ( $num != $prime[-1] ) {
    @prime = grep {
        ($_ <= $num) or (($_ % $num) != 0);
    } @prime;

    $num = bigger_number( $num, \@prime );
}

# while ( my @numbers = splice(@prime, 0, 8) ) {
#     say join( ',', map sprintf("%5d", $_), @numbers );
# }

# exit 0;

{
    my $fs = 48_000;
    my @delay_samples = map {
        find_near_number( int($fs * $_ / 1_000), \@prime );
    } ( 35.2, 36.7, 37.9, 39.7 );

    say join( ', ', map sprintf("%4d", $_), @delay_samples );

    my $n = 48;
    my @params = map {
        my $t = $_ / $n;
        my $time_sec = 2 ** (1 + (5.5 * $t));

        my $samples = int($fs * $time_sec);
        my @times_list = map {
            $samples / $_;
        } @delay_samples;

        # printf("[%2d] %5.2f\n", $i, $time_sec);

        my @gain_list = calc_gain( @times_list );

        +{
            time_sec => $time_sec,
            gain_list => \@gain_list
        };
    } 1..$n;

    for (my $i=0; $i<scalar(@params); $i++) {
        my $j = $i + 1;
        my $param = $params[$i];
        my $gain_csv = join( ', ', map {
            my $gain = $_;
            sprintf("0x%04X", int(($gain * 0x8000) + .5));
        } @{$param->{gain_list}} );

        printf("  { %s }, // [%2d] %.3f\n",
            $gain_csv, $j, $param->{time_sec});
    }

    for (my $i=0; $i<scalar(@params); $i++) {
        my $j = $i + 1;

        my $param0 = $params[0];
        my @gain_list0 = @{$param0->{gain_list}};

        my $param = $params[$i];
        my @gain_list = @{$param->{gain_list}};

        my @list = map calc_zmax($_), @gain_list;

        #my $tmp = $list[-1];
        my $gain_csv = join( ', ', map {
            sprintf("%6.2f", $_);
        } @list );

        printf("  { %s }, // %2d\n", $gain_csv, $j);
    }

    if (0) {
        my $i = scalar(@params) - 1;
        my $j = $i + 1;

        my $param0 = $params[0];
        my @gain_list0 = @{$param0->{gain_list}};

        my $param = $params[$i];
        my @gain_list = @{$param->{gain_list}};

        my @list = map calc_zmax($_), @gain_list;

        #my $tmp = $list[-1];
        my $gain_csv = join( ', ', map {
            sprintf("%6.2f", $_);
        } @list );

        printf("  { %s }, // %2d\n",
            $gain_csv, $j);
    }

    sub calc_gain {
        my @times_list = @_;
        return map {
            2 ** (log2(0.001) / $_);
        } @times_list;
    }

    sub calc_zmax {
        my $gain = shift;
        return 1.0 / (1.0 - $gain);
    }
}

sub find_near_number {
    my ( $num, $numbers ) = @_;

    my $n = scalar(@{$numbers});
    for (my $i=0; $i<$n; $i++) {
        if ( $num < $numbers->[$i] ) {
            my $diff_over = $numbers->[$i] - $num;
            my $diff_under = $num - $numbers->[$i-1];

            if ( $diff_over < $diff_under ) {
                return $numbers->[$i];
            }
            else {
                return $numbers->[$i-1];
            }
        }
    }

    die "near value not found: ", $num;
}

sub bigger_number {
    my ( $th, $numbers ) = @_;
    my @tmp = grep { $th < $_; } @{$numbers};
    return @tmp ? $tmp[0] : $th;
}

sub log2 {
    return log($_[0]) / log(2.0);
}

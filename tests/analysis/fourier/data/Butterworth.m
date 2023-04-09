% This script is intended to create the reference values for the filter tests

clc; clear; close all;
# pkg install "https://downloads.sourceforge.net/project/octave/Octave%20Forge%20Packages/Individual%20Package%20Releases/control-3.4.0.tar.gz"
# pkg install "https://downloads.sourceforge.net/project/octave/Octave%20Forge%20Packages/Individual%20Package%20Releases/signal-1.4.3.tar.gz"

pkg load signal

fs = 10000; % [Hz] Sample frequency
fc = 100; % [Hz] cut off frequency
fsignal = 100; % [Hz] Signal frequency
x = 0 : 1/fs : 1/fsignal * 10; % 10 periods
y = sin(x * 2*pi * fsignal);

[b, a] = butter(1,fc/(fs/2), "low");
filtered_data = filter(b,a,y);
csvwrite("butterworth.csv", [x; y; filtered_data]);

% plotting
freqz(b,a,[],fs)

figure();
hold on;
plot(x, y,'DisplayName','original');
plot(x, filtered_data,'DisplayName','filtered');
legend();

%% Explanation
% This code reads data from 4 impulse vibration tests performed with 5 measurement nodes
% and computes the sampling frequency of each acceleration value in order to analyse the 
% sampling frequency stability of the low cost system

% The data from the 4 impulse vibration tests used in this study is located 
% in 5 .xlsx files, respective to the 5 measurement nodes, and need to be
% contained in the root folder of this code
%
% In each file (that refers to a measurement node) there are 4 tabs named
% "8L", "9L", "10L", "11L", which are respective to each of the 4 impulse vibration tests
% For each node and each test, all acceleration values are time stamped, which allows
% computing the sampling frequency of each sample
%
% In each tab of each file, the first column is the time stamp in microseconds,
% which refers to the MCU's internal clock.
% The second column refers to the relative time respective to the beginning of the series
% The third column refers to the time in seconds
% The fourth column refers to the acceleration value in bits. To convert from bits
% to equivalent g's (gravity acceleration), the scale factor is 16384 (as per MPU6050 configuration)

clear all

%% Looping definition
%Initital declarations
archives = ["NODE_3";...
            "NODE_4";...
            "NODE_2";...
            "NODE_5";...
            "NODE_1"];
sheet2read = ["8L";"9L";"10L";"11L"];
time=zeros(length(archives),length(sheet2read),20000);

% Read all data and store the time data in the "time" variable
% For the sampling frequency stability study, only time matters (we dont care for acceleration)
for archiveIteration=1:length(archives)
    for sheetIteration=1:length(sheet2read)
        %% Spreadsheet import
        %Import options
        clearvars -except time archives sheet2read archiveIteration sheetIteration
        numVars=4;
        opts = spreadsheetImportOptions('NumVariables',numVars);
        opts.Sheet = sheet2read(sheetIteration);
        opts.VariableNames(1) = {'absoluteTime'};
        opts.VariableNames(2) = {'microsecTimeZeroShifted'};
        opts.VariableNames(3) = {'secTimeZeroShifted'};
        opts.VariableNames(4) = {'acceleration'};
        %Import spreadsheet to M variable
        M = readmatrix(archives(archiveIteration)+".xlsx",opts);
        %Extract time and acceleration from M (they are the third and fourth collumns)
        for i=1:length(M)
            time(archiveIteration,sheetIteration,i)=str2num(cell2mat(M(i,2)));
        end
    end
end

%Variable to store the time interval
timeInterval=zeros(length(time(:,1,1)),length(time(1,:,1)),length(time(1,1,:)));

%Compute time interval between consecutive samplings
for i=1:length(time(:,1,1))
    for j=1:length(time(1,:,1))
        for k=2:length(time(1,1,:))
            timeInterval(i,j,k)=time(i,j,k)-time(i,j,k-1);
        end
    end
end

samplingFrequency=1./(timeInterval./1000000);

%% Plot histogram
figure
% k=histogram(samplingFrequency(isfinite(samplingFrequency)),'Normalization','probability')
k=histogram(samplingFrequency(isfinite(samplingFrequency)),'BinWidth',0.3,'Normalization','probability')
xlabel("Frequency (Hz)")
ylabel("Probability (-)")
xlim([590 620])
xlim
grid on
hold on
yyaxis right
ranges=k.BinEdges;
ys=k.Values;

bins=zeros(1,length(ranges)-1);
for i=2:length(ranges)
    bins(i-1)=(ranges(i)+ranges(i-1))/2;
end

plot(bins,cumsum(ys),'-x');
legend("Probability","Accumulated probability")
hold on

% create smaller axes in top right, and plot on it
axes('Position',[.6 .5 .25 .25])
box on
% k=histogram(samplingFrequency(isfinite(samplingFrequency)),'BinWidth',0.3,'Normalization','probability')
k=histogram(samplingFrequency(isfinite(samplingFrequency)),'Normalization','probability')
% xlabel("Frequency (Hz)")
% ylabel("Probability (-)")
xlim
grid on
hold on
yyaxis right
ranges=k.BinEdges;
ys=k.Values;
% xlim([590 620])

bins=zeros(1,length(ranges)-1);
for i=2:length(ranges)
    bins(i-1)=(ranges(i)+ranges(i-1))/2;
end

plot(bins,cumsum(ys));
hold off

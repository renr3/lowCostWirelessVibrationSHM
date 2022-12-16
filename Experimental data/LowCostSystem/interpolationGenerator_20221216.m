%% Explanation
% This code reads each raw measurement session file of each node obtained from the low-cost system and outputs a single file
% containing interpolation results of all nodes (A1 to A5)
% The raw measurement files need to be located in a folder called "RawData" in the root folder containing this code
% You have to specify the new sampling frequency desired (fs variable) and the new total time of the measurement session (totalTime variable)
% This file can be directly used with the supplied files with the paper
% It outputs files for all interpolation methods
% It also outputs a file with the raw measurements
% The generated files are stored in a folder called "InterpolatedData", created in the root folder containing this code

%% Code: Looping definition
%Switch between the two variables below to interpolate impulse or ambient vibration data
%fileNames=["NODE_3","NODE_4","NODE_2","NODE_5","NODE_1"];
%fileEnum=["-ImpulseVibration_LowCost"];
fileNames=["NODE_3","NODE_4","NODE_2","NODE_5","NODE_1"];
fileEnum=["-AmbientVibration_LowCost"];

for archiveIteration=1:length(fileNames)
    for sheetIteration=1:length(fileEnum)
        %% Spreadsheet import
        %Import options
        clearvars -except fileNames fileEnum archiveIteration sheetIteration
        C=table2cell(readtable('RawData/'+fileNames(archiveIteration)+fileEnum(sheetIteration)+'.csv','Format','%s%s%s%s')) ;
        M=cellfun(@str2num,C);
        numVars=4;
        %Extract time and acceleration from M (they are the third and fourth collumns)
        acceleration=M(:,4);
        time=[0; M(2:end,1)-M(1:end-1,1)];
        time(time<0) = 1870; %When we append two series, the time step above will be zero
                             %because the clock restart. In this case, we just substitute 
                             %the value by 1870, which is an abritrary time step
        time=cumsum(time);
        time =time./1000000;
        
        %% Interpolation of the vector
        %Interpolation sampling frequency
        fs = 250;
        %TotalTtime of the interpolation vector, in seconds. Better if lower than the duration of the shorter
        %For impulse vibration, use 25 seconds
        %For ambient vibration, use 18 minutes, i.e., 18*60 seconds
        totalTime = 25; %Impulse vibration data
        %totalTime = 18*60; %Ambient vibration data
        %Build the query point vectors of the interpolation
        xq = [0:1/(fs):totalTime];
        %Build the interpolation point matrix, to store various interpolation schema results
        accelIntp = zeros(4,length(xq));
        interpolationSchema = ["linear","spline","pchip","makima"];
        for i=1:4
            accelIntp(i,:) = interp1(time,detrend(acceleration)/16384,xq,interpolationSchema(i));
        end

        %% Frequency domain visualization
        %Raw signal FFT
        accel=detrend(acceleration)/16384;
        L = length(accel); 
        fftRaw = fft(accel);
        fftTwoSided = abs(fftRaw/L); 
        fftOneSided = fftTwoSided(1:(L/2+1));
        fftOneSided(2:end-1) = 2*fftOneSided(2:end-1);
        fftFreq = (length(acceleration)/time(end))*(0:(L/2))/L;

        %Interpolated signals FFT
        L = length(accelIntp); 
        fftIntp = fft(accelIntp.');
        fftIntp = fftIntp.';
        fftIntpTwoSided = abs(fftIntp/L); 
        fftIntpOnesided = fftIntpTwoSided(:,1:(L/2+1));
        fftIntpOnesided(:,2:end-1) = 2*fftIntpOnesided(:,2:end-1);
        fftIntpOneFreq = fs*(0:(L/2))/L;

        % Raw data NUFFT
        fftNUFFT = nufft(accel,time,fftIntpOneFreq);
        LNUFFT=length(fftNUFFT);

        %% Phase angle visualization
        %Interporlation signals
        fftIntpTemp=fftIntp;%store the FFT results in another array
        %detect noise (very small numbers (eps)) and ignore them
        tol = 1e-6;
        fftIntpTemp(abs(fftIntpTemp)<tol) = 0;
        %maskout values that are below the threshold
        fftIntpTemp(abs(fftIntpTemp)<tol) = 0; 
        theta=angle(fftIntpTemp)*180/pi; %phase information

        %FFT signal
        fftRawTemp=fftRaw;%store the FFT results in another array
        %detect noise (very small numbers (eps)) and ignore them
        tol = 1e-6;
        fftRawTemp(abs(fftRawTemp)<tol) = 0;
        %maskout values that are below the threshold
        fftRawTemp(abs(fftRawTemp)<tol) = 0; 
        thetaRaw=angle(fftRawTemp)*180/pi; %phase information

        % NUFFT signal
        fftNUFFTTemp=fftNUFFT;%store the FFT results in another array
        %detect noise (very small numbers (eps)) and ignore them
        tol = max(abs(fftNUFFTTemp))/10000;
        fftNUFFTTemp(abs(fftNUFFTTemp)<tol) = 0;
        thetaNUFFT=angle(fftNUFFTTemp)*180/pi; %phase information

        %% Inverse FFT to time domain
        %Compute time data from interpolation
        accelFromIntpFFTInverse=ifft(fftIntp,[],2); 
        timeFromIntpFFTInverse=[0:1:length(accelFromIntpFFTInverse)-1]*(1/(fftIntpOneFreq(end)*2));
        %Compute time data from FFT
        accelFromFFTInverse=ifft(fftRaw); 
        timeFromFFTInverse=[0:1:length(accelFromFFTInverse)-1]*(1/(fftFreq(end)*2));
        %Compute time data from NUFFT
        accelFromNUFFTInverse=ifft([real(fftNUFFT(1));(fftNUFFT(2:end))./2;flip(conj(fftNUFFT(2:end)./2))]); 
        % accelFromNUFFTInverse=ifft(fftNUFFT); 
        timeFromNUFFTInverse=[0:1:length(accelFromNUFFTInverse)-1]*(1/(fftIntpOneFreq(end)*2));
        
        %% Save data to Excel spreadsheet files
        interpolationMethod = ["Raw","NUFFT","Linear","Spline","PChip","MAkima"];
        columnRange = ["A2","B2","C2","D2","E2"];
        header = ["NODE_3","NODE_4","NODE_2","NODE_5","NODE_1"];
        headerRange = ["A1","B1","C1","D1","E1"];
        if ~exist("InterpolatedData", 'dir')
            mkdir("InterpolatedData")
        end
        for j=1:length(interpolationMethod)
            %Use this if dealing with small time series (impulse)
            filename = "InterpolatedData/Int"+fileEnum(sheetIteration)+"_"+interpolationMethod(j)+".xlsx";
            switch j
                case 1
                    temp=detrend(acceleration)/16384;
                    writematrix(header(archiveIteration),filename,'Sheet',interpolationMethod(j),'Range',headerRange(archiveIteration),'UseExcel',false);
                    writematrix(temp,filename,'Sheet',interpolationMethod(j),'Range',columnRange(archiveIteration),'UseExcel',false);
                case 2
                    writematrix(header(archiveIteration),filename,'Sheet',interpolationMethod(j),'Range',headerRange(archiveIteration),'UseExcel',false);
                    writematrix(accelFromNUFFTInverse,filename,'Sheet',interpolationMethod(j),'Range',columnRange(archiveIteration),'UseExcel',false);
                otherwise
                    writematrix(header(archiveIteration),filename,'Sheet',interpolationMethod(j),'Range',headerRange(archiveIteration),'UseExcel',false);
                    writematrix(accelFromIntpFFTInverse(j-2,:).',filename,'Sheet',interpolationMethod(j),'Range',columnRange(archiveIteration),'UseExcel',false); 
            end
        end
    end
    disp("Node analysed:" + fileNames(archiveIteration))
end

load gong
sound(y,Fs)


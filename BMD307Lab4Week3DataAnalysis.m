clear
clc
close all

load('BMD307Lab4Week3FullData.mat')
light_reaction_time=zeros(size(full_light_data,1),size(full_light_data,2));
sound_reaction_time=zeros(size(full_light_data,1),size(full_light_data,2));
fs = 1000; fo = 60; q = 35;

d60 = designfilt('bandstopiir','FilterOrder',2, ...
               'HalfPowerFrequency1',59,'HalfPowerFrequency2',61, ...
               'DesignMethod','butter','SampleRate',fs);
           
d120 = designfilt('bandstopiir','FilterOrder',2, ...
               'HalfPowerFrequency1',119,'HalfPowerFrequency2',121, ...
               'DesignMethod','butter','SampleRate',fs);
       
d180 = designfilt('bandstopiir','FilterOrder',2, ...
               'HalfPowerFrequency1',179,'HalfPowerFrequency2',181, ...
               'DesignMethod','butter','SampleRate',fs);
           
for a=1:size(full_light_data,1)
    for b=1:size(full_light_data,2)
    light_data=abs(full_light_data{a,b}(:,1));  
    sound_data=abs(full_sound_data{a,b}(:,1));
    
    %Filtering out 60 Hz Harmonics out of light response data
    filt_light_data=light_data;
    filt_light_data=filtfilt(d60,light_data);
    filt_light_data=filtfilt(d120,filt_light_data);
    filt_light_data=filtfilt(d180,filt_light_data);
    
    %Filtering out 60 Hz Harmonics out of sound response data
    filt_sound_data=sound_data;
    filt_sound_data=filtfilt(d60,sound_data);
    filt_sound_data=filtfilt(d120,filt_sound_data);
    filt_sound_data=filtfilt(d180,filt_sound_data);
    
    filt_light_data_rms=BMD307Lab4_RMS(filt_light_data,50);
    filt_light_data_rms=filt_light_data_rms-mean(filt_light_data_rms(1:100));
    filt_sound_data_rms=BMD307Lab4_RMS(filt_sound_data,50);
    filt_sound_data_rms=filt_sound_data_rms-mean(filt_sound_data_rms(1:100));
    
    light_baseline_std=std(filt_light_data_rms(1:100));
    light_signal_locs=find(filt_light_data_rms>6.*light_baseline_std);
    if ~isempty(light_signal_locs)
        light_reaction_time(a,b)=min(light_signal_locs);
    end
    sound_baseline_std=std(filt_sound_data_rms(1:100));
    sound_signal_locs=find(filt_sound_data_rms>6.*sound_baseline_std);
    if ~isempty(sound_signal_locs)
        sound_reaction_time(a,b)=min(sound_signal_locs);
    end
    
    reaction_difference(a,b)=light_reaction_time(a,b)-sound_reaction_time(a,b);

    end
end
%Eliminating invalid data points
sound_reaction_time(1,:)=[];
light_reaction_time(1,:)=[];

sound_reaction_time=sound_reaction_time';
light_reaction_time=light_reaction_time';

for a=1:size(sound_reaction_time,2)
sound_invalids=find(sound_reaction_time(:,a)<100 | sound_reaction_time(:,a)>750);
light_invalids=find(light_reaction_time(:,a)<100 | light_reaction_time(:,a)>750);
sound_valids=find(sound_reaction_time(:,a)>100 & sound_reaction_time(:,a)<750);
light_valids=find(light_reaction_time(:,a)>100 & light_reaction_time(:,a)<750);
valid_sound_mean=mean(sound_reaction_time(sound_valids,a));
valid_light_mean=mean(light_reaction_time(light_valids,a));
sound_reaction_time(sound_invalids,a)=valid_sound_mean;
light_reaction_time(light_invalids,a)=valid_light_mean;
light_means(a)=valid_light_mean;
sound_means(a)=valid_sound_mean;
end


n=1;
for a=1:2
    for b=1:size(sound_reaction_time,2)
        for c=1:size(sound_reaction_time,1)
            stimulus(n,1)=a-1;
            if a==1
                reaction(n,1)=light_reaction_time(c,b);
            elseif a==2
                reaction(n,1)=sound_reaction_time(c,b);
            end
            subject(n,1)=b;
            n=n+1;
        end
    end
end
            
data_table=table(stimulus,subject,reaction);
model=fitlme(data_table,'reaction~stimulus','FitMethod','ML');
model_subjectblock=fitlme(data_table,'reaction~stimulus+(1|subject)','FitMethod','ML');
disp(model)
disp(model_subjectblock)
compare(model,model_subjectblock)

figure()
plot(light_reaction_time,'o')
xlabel('Trial Number')
ylabel('Reaction Time (ms)')
title('Second Study Without Confounding Subject: Reaction Time to a Visual Stimulus')
legend('Subject 2','Subject 3','Subject 4','Subject 5','Subject 6')
ylim([0,800])

figure()
plot(sound_reaction_time,'o')
xlabel('Trial Number')
ylabel('Reaction Time (ms)')
title('Second Study Without Confounding Subject: Reaction Time to an Auditory Stimulus')
legend('Subject 2','Subject 3','Subject 4','Subject 5','Subject 6')
ylim([0,800])

reaction_time_matrix=[light_reaction_time;sound_reaction_time];
[p,tbl]=anova2(reaction_time_matrix,40);

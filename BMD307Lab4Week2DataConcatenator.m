clear
clc
close all

full_light_data=cell(6,40);
full_sound_data=cell(6,40);

load('BMD307Lab4Week3Abdalla.mat');
for a=1:40
full_light_data{1,a}=light_data{a};
full_sound_data{1,a}=sound_data{a};
end
load('BMD307Lab4Week3Deirdre.mat')
for a=1:40
full_light_data{2,a}=light_data{a};
full_sound_data{2,a}=sound_data{a};
end
load('BMD307Lab4Week3Gabe.mat')
for a=1:40
full_light_data{3,a}=light_data{a};
full_sound_data{3,a}=sound_data{a};
end
load('BMD307Lab4Week3Roland.mat')
for a=1:40
full_light_data{4,a}=light_data{a};
full_sound_data{4,a}=sound_data{a};
end
load('BMD307Lab4Week3Sam.mat')
for a=1:40
full_light_data{5,a}=light_data{a};
full_sound_data{5,a}=sound_data{a};
end
load('BMD307Lab4Week2Zeri.mat')
for a=1:35
full_light_data{6,a}=light_data{a};
full_sound_data{6,a}=sound_data{a};
end
for a=36:40
full_light_data{6,a}=zeros(size(light_data{a-10},1),1);
full_sound_data{6,a}=zeros(size(light_data{a-10},1),1);
end

clearvars -except full_light_data full_sound_data
save('BMD307Lab4Week3FullData')
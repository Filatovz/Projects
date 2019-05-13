clear
clc
close all

ntests=input('How many people will be tested?');
ntrials=input('How many trials per person will be conducted?');

nScopeAPI('setP1on',0);
nScopeAPI('setA1on',0);

for a=1:ntests
    for b=1:ntrials
        retry='n';
        while retry=='n'
        t_wait=randi([3,7],1,1);
        disp(['Light Test: Person ' num2str(a) ', Trial ' num2str(b) '. Press any key to continue...'])
        pause
        disp('Prepare to flex...')
        pause(t_wait)
        nScopeTurnOnP1(100,0.99)
        light_data{a,b}=nScopeReadAllChannels(1000,1500,[1,1,1,1]);
        nScopeAPI('setP1on',0);
        retry=input('Continue to next trial? Type "n" to retry...','s');
        end
    end
    
    for b=1:ntrials
        retry='n';
        while retry=='n'
        t_wait=randi([3,7],1,1);
        disp(['Sound Test: Person ' num2str(a) ', Trial ' num2str(b) '. Press any key to continue...'])
        pause
        disp('Prepare to flex...')
        pause(t_wait)
        nScopeTurnOnA1(440,0,4)
        sound_data{a,b}=nScopeReadAllChannels(1000,1500,[1,1,1,1]);
        nScopeAPI('setA1on',0);
        retry=input('Continue to next trial? Type "n" to retry...','s');
        end
    end
end 

save('BMD307Lab4Week3Gabe')
            
        
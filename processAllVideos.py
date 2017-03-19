#coding: utf-8

import os
import sys
from multiprocessing import Pool

def compute_features(video):

	dir_out = '/media/carlos/Data/graduacao/8-periodo/vc/Features/'
#	dir_out = './Testando/'
	label = video.split(' ')[0].split('/')[-2]
	type_video = video.split(' ')[0].split('/')[-3]
	count = int(video.split(' ')[1])
	
	os.system("mkdir -p %s/%s/flow_x/%s/%d"    % (dir_out, type_video, label, count))
	os.system("mkdir -p %s/%s/flow_y/%s/%d"    % (dir_out, type_video, label, count))
	os.system("mkdir -p %s/%s/grad_x/%s/%d"    % (dir_out, type_video, label, count))
	os.system("mkdir -p %s/%s/grad_y/%s/%d"    % (dir_out, type_video, label, count))
	os.system("mkdir -p %s/%s/grayscale/%s/%d" % (dir_out, type_video, label, count))
	print video
	c = "./detectPeople %s %s/%s %s" % (video.split(' ')[0], dir_out, type_video, str(count))		
#		print c
	os.system(c)
			
f = open('path_videos.txt')

l = f.readlines()

for i in range(len(l)):
	l[i] = l[i].split('\n')[0].split(' ')[0] + ' ' + l[i].split('\n')[0].split(' ')[1]

p = Pool(4)
'''
compute_features(l[0])
compute_features(l[1])
compute_features(l[2])
compute_features(l[3])
compute_features(l[4])
'''
p.map(compute_features, l)

#compute_features(dataset[1], out[1])

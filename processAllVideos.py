#coding: utf-8

import os
import sys

def compute_features(videos_list, dir_out):
	

	
	for video in videos_list:
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
				
	'''		
	for label in os.listdir(path_database):
		if (label != 'walking'): continue
		path_label = "%s/%s" % (path_database, label)
		count = 0 
		for video in os.listdir(path_label):

			os.system("mkdir -p %s/flow_x/%s/%d"    % (path_out, label, count))
			os.system("mkdir -p %s/flow_y/%s/%d"    % (path_out, label, count))
			os.system("mkdir -p %s/grad_x/%s/%d"    % (path_out, label, count))
			os.system("mkdir -p %s/grad_y/%s/%d"    % (path_out, label, count))
			os.system("mkdir -p %s/grayscale/%s/%d" % (path_out, label, count))
			
			path_video = "%s/%s" % (path_label, video)
			dir_video = "%d" % (count)
			c = "./detectPeople %s %s %s" % (path_video, path_out, dir_video)
			
			os.system(c)
			print path_video
				
			count+=1
	'''
f = open(sys.argv[1])

l = f.readlines()

for i in range(len(l)):
	l[i] = l[i].split('\n')[0].split(' ')[0] + ' ' + l[i].split('\n')[0].split(' ')[1]

compute_features(l, sys.argv[2])
#compute_features(dataset[1], out[1])

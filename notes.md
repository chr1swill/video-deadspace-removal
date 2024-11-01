# Deadspace removal pipe line

need to be modified to be more cohesive, like a single step you pass in the video and we handle the rest.

then i can worry about makeing it faster and easier to run on my mac without having to worry about it. 

## scirpt used to break video into files
ffmpeg -i input.mkv ./output/frames%06d.png

## run through c program
sh build.sh && ./main <dir_containing_images_files>

## script use to re assembly files into video again
 ffmpeg -framerate 60 -start_number 1 -i output_renamed/frames%06d.png -c:v libx264 -pix_fmt yuv420p video/output.mkv

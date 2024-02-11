#!/bin/bash

export PATH=${PATH}:'/c/Program Files/Inkscape/bin'

function svg2png () {
  src=confluence.svg
  dim=$1x$1
  dst=app_${dim}.png
  echo "Generating icon ${dst} from ${src}"
  convert  -background none -resize ${dim} ${src} ${dst}
}

function svg2ico () {
  src=confluence.svg
  dim=$1x$1
  dst=app_${dim}.ico
  echo "Generating icon ${dst} from ${src}"
  convert  -background none -resize ${dim} ${src} ${dst}
}

if false
then
svg2png 16
svg2png 32
svg2png 48
svg2png 64
svg2png 96
svg2png 128
svg2png 256
convert app_*.png app.ico
fi

#convert -density 64x64 -background transparent confluence.svg -define icon:auto-resize -colors 16 app.ico
#convert -density 64x64 -background transparent confluence.svg -colors 16 app.ico
rm -f app.ico
convert -background transparent confluence.svg app.ico
#convert -background none +dither -colors 16 -depth 4 confluence.svg app.ico

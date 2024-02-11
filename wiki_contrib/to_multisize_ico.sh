#!/bin/bash

export PATH=${PATH}:'/c/Program Files/Inkscape/bin'

function svg2png () {
  sz=$1
  convert  -background none -resize ${sz}x${sz} confluence.svg app_${sz}.png
}

function svg2ico () {
  sz=$1
  convert  -background none -resize ${sz}x${sz} confluence.svg app_${sz}.ico
}

svg2ico 16
svg2ico 32
svg2ico 64
svg2ico 96
svg2ico 128
svg2ico 256
#convert app_*.png app.ico


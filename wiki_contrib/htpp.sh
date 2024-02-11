#!/bin/bash

export SRC=$1
export DST=$2

JSCK=jshint
JSMIN='html-minifier --collapse-whitespace --remove-comments --remove-optional-tags --remove-redundant-attributes --remove-script-type-attributes --remove-tag-whitespace --use-short-doctype --minify-css true --minify-js true --continue-on-parse-error'

# RegExp to find Key words
re_inc_css='<link rel="stylesheet"(.*)href="(.*)">'

re_inc_js='<script type="text/javascript" src="(.*)"></script>'
re_inc_js2='<script src="(.*)"></script>'
re_inc_jscom='//#include (.*)'
re_inc_svg='<img(.*) src="(.*\.svg)"(.*)>'
re_inc_img='(.*) src=[^"]*"([^"]*\.(png|jpg|gif))"(.*)'

include_css () {
  if which csstidy >/dev/null 2>&1; then
    echo "CHECKING CSS FILE $2"
    csstidy $2 >/dev/null 2>&1 || exit 1
  fi

  echo "INCLUDING CSS $2"
  ( echo "<style $1>"; cat $2; echo '</style>' ) >>$3
}

include_js () {
  if which jshint >/dev/null 2>&1; then
    echo "CHECKING JS FILE $1"
    ${JSCK} $1 || true
  fi

  echo "INCLUDING JS $1"
#  sed 's?//.*??' $1 >$1.nocom
#  MIN=$(${JSMIN} $1.nocom)
#  rm -f $1.nocom
#  [ -n "$3" ] && ( echo "<$3>${MIN}</$3>" ) >>$2 || echo ${MIN} >>$2

  [ -n "$3" ] && ( echo "<$3>"; cat $1; echo "</$3>" ) >>$2 || cat $1 >>$2
}

include_img () {
  echo "INCLUDING $3 image '$2'"

  # Tentative d'optimisation des png et jpeg
  cp "$2" img$$

  case $3 in
    png)
      optipng.exe -force -strip all -o7 img$$
      ;;
    jpg)
      jpegoptim.exe -f -s img$$
      ;;
    *)
      ;;
  esac

  b64_img=$(base64 -w 0 img$$)
  rm -f img$$
  echo "$1 src=\"data:image/$3;base64,${b64_img}\" $4" >>$5
}

include_svg () {
  echo "INCLUDING SVG vector image '$2'"
  svg_img=$(sed 's/^<svg//' $2)
  echo "<svg $1 $3 ${svg_img}" >>$4
}

#tidy $SRC

while read ln; do
  # Remove <cr>
  ln=${ln%$'\r'}
  
  if [[ $ln =~ $re_inc_css ]]; then
    include_css "${BASH_REMATCH[1]}" "${BASH_REMATCH[2]}" "$DST"
    continue
  fi

  if [[ $ln =~ $re_inc_js ]] || [[ $ln =~ $re_inc_js2 ]]; then
    include_js "${BASH_REMATCH[1]}" "$DST" script
    continue
  fi

  if [[ $ln =~ $re_inc_jscom ]]; then
    include_js "${BASH_REMATCH[1]}" "$DST"
    continue
  fi

  if [[ $ln =~ $re_inc_svg ]]; then
    include_svg "${BASH_REMATCH[1]}" "${BASH_REMATCH[2]}" "${BASH_REMATCH[3]}"  "$DST"
    continue
  fi

  if [[ $ln =~ $re_inc_img ]]; then
    include_img "${BASH_REMATCH[1]}" "${BASH_REMATCH[2]}" "${BASH_REMATCH[3]}" "${BASH_REMATCH[4]}" "$DST"
    continue
  fi

  echo $ln  | sed 's/\\/\\\\/g' >>$DST
done <$SRC

export PUT_TAG_HERE=$(git describe --tags --abbrev=0 2>/dev/null || echo "NOT A TAGGED VERSION")
export PUT_COMMIT_HERE=$(git rev-parse --short HEAD)
envsubst < $DST > ${DST}.tagged
mv -f ${DST}.tagged ${DST}

#tidy $DST


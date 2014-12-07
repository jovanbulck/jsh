## a file containing some test commands to be executed by jsh each time
## Travis CI triggers a new build

echo -e "\\n----------- START OF JSH_TRAVIS_TEST_SCRIPT -----------\\n"

echo Hi I am running jsh --version :
echo "--------------------------"
./jsh --version
echo -e "--------------------------\\n"

alias grep "grep --color=auto -i"
alias ll    ls\ -lh      # some inline comment
echo -e "this is the output of alias:\\n--------------------------" ; alias
echo -e "--------------------------\\n"

echo "this is the output of \"ll | grep alias\"" && ll | grep alias

echo -e "\\nthis is the output of \"ll | grep alias | wc\"" && ll | grep alias | wc

# a long comment: loooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooo ooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong

echo -e "\\n----------- END OF JSH_TRAVIS_TEST_SCRIPT -----------\\n"

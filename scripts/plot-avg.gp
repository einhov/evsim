set term wxt

FILE = ARG1
plot for [num=1:7] FILE.num."/h/scores.avg" using 1:2 with lines ti FILE.num noenhanced lw 4

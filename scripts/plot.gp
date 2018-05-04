set term wxt

FILE = ARG1
plot FILE."/h/scores" using 1:4 with lines ti FILE noenhanced, FILE."/h/scores.avg" using 1:2 with lines lw 4 ti ""

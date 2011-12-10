def parse_color(s):
    color = s[:11].split();
    name = s[11:-1].replace('\t','');
    return color + [name];

r = open('rgb.txt', 'r');
w = open('rgb.tex', 'w');

for line in r:
    row = "\\colorrow"
    for c in parse_color(line):
        row += '{' + c + '}'
    row += "\n"
    w.write(row);


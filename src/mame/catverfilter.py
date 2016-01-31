good_clones = ["pacman","maketrax","800fath","fastfred","rugrats","terrafu","galagamf",
               "galaga3","dkongpe","rbtapper","atompunk",",newapunk","perfrmanu","pyros","astormu",
               "simpsons2p","vendetta2p","tmnt2po","punkshot2","tmnt22pu","ssridersubc","xmen2pu","xmen6p",
               "gauntlet2p","gaunt22p","cyberbal2p","rampart2p","mspacmnf","pacmanf"]

badRoms = []
goodRoms = {}
badLines = []
# clean_cat = open("catverclean.ini", "w")
for line in open("catver.ini", "r"):
    romName = line[0:line.find("=")]
    if line.find("* Mature *") > 0 or \
                    line.find("Mahjong") > 0 or \
                    line.find("=Casino") > 0 or \
                    line.find("=Electromechanical") > 0 or \
                    line.find("=Quiz") > 0 or \
                    line.find("=Utilities") > 0:
        badLines.append(line)
        badRoms.append(romName)
    else:
        goodRoms[romName] = line[line.find("=")+1:line.find("\r\n")]
    #     clean_cat.write(line);
print "finished catver"

machineInfo = []
clones = []
descriptions = {}
years = {}
last_name = ""
for line in open("mame.xml", "r"):
    if line.find("<machine") > 0:
        partial = line[line.find("name=\"")+6:]
        name = partial[:partial.find("\"")]
        if name in badRoms:
            continue
        if line.find("cloneof") > 0 and name not in good_clones:
            clones.append(name)
        else:
            last_name = name
    if len(last_name) > 0 and line.find("<description>") > 0:
        descriptions[last_name] = line[line.find(">")+1:line.find("</")]
    if len(last_name) and line.find("<year>") > 0:
        years[last_name] = line[line.find(">")+1:line.find("</")]
        last_name = ""
print "finished xml"

def is_clone(name):
    if name in good_clones:
        return False
    return name in clones

f = open("arcade.lst.txt", "r")
new_file = open("arcadefiltered.lst", "w")

i = 0;
for line in f:
    if line.find("\t\t\t\t") == 0:
        continue
    if line.find("//") == 0 and len(line) > 2 and line[2:3].isalnum():
        continue
    first = line[0:1];
    if not first.isalnum():
        new_file.write(line)
        continue
    i += 1
    if i % 100 == 0:
        print str(i)+" lines"
    space = line.find(" ")
    no_comment = False
    if(space > 0):
        name = line[0:space]
    else:
        name = line[0:line.find("\r\n")]
    if name in badRoms or is_clone(name):
        continue

    new_line = name
    while(len(new_line) < 16):
        new_line += " "
    new_line += "// "
    if name in years:
        new_line += years[name]

    while(len(new_line) < 25):
        new_line += " "

    if name in goodRoms:
        new_line += goodRoms[name]

    while(len(new_line) < 60):
        new_line += " "

    if name in descriptions:
        new_line += descriptions[name]

    new_line += "\r\n"
    new_file.write(new_line)
print "done"
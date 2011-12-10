import os

def calcCompessionRatio(uncompressedSize,compressedSize):
    """ Calculates the compression ratio of two file sizes.
    """
    return round((float(compressedSize) / uncompressedSize) * 100,1)

testDirectory= "../testfiles/"

out = open('table.tex', 'w')

newTableLine = """\\\\
"""

tableFirst ="""\\noindent\\makebox[\\textwidth]{%
\\begin{tabular}{"""

out.write(tableFirst)

out.write('l' * (2 + (15 - 9 + 1)))

out.write("""}
\\toprule
File & Original size & """)

for codeSize in range(9,16):
    out.write(str(codeSize) + \
              ( '-bit codes(\%) & ' if codeSize != 15 else '-bit codes(\%) ' + newTableLine ))

tableEnd = """\\bottomrule
\\end{tabular}}"""

dirList = os.listdir(testDirectory)

for fileName in dirList:

    if fileName[-4:] != '.lzw':
        out.write(fileName + ' & ')

        fileName = os.path.join(testDirectory,fileName)
        uncompressedSize = os.path.getsize(fileName)
        out.write(str(uncompressedSize) + ' & ')

        for codeSize in range(9,16):

            command = './lzw ' + '-cs=' + str(codeSize) + ' ' + fileName
            os.system(command)

            compressedSize = os.path.getsize(fileName + '.lzw')
            #            print "Compressed size: " + str(compressedSize)

            out.write(str(calcCompessionRatio(uncompressedSize,compressedSize)) + \
                      (' & ' if codeSize != 15 else newTableLine))
            # delete the outputted file.
            os.remove(fileName + '.lzw')

out.write(tableEnd)

out.close()

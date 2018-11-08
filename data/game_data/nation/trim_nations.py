import sys, os


def main ():
	for sFile in sys.argv [1:]:
		file = open (sFile)
		lstLines = map (lambda x: x.rstrip (), file.readlines ())
		file.close ()

		file = open (sFile, 'w')
		bInLegend = False
		for sLine in lstLines:
			if True == sLine.startswith ('legend=_("'):
				if '")' not in sLine:
					bInLegend = True
				file.write ('legend=_("")\n')
				continue

			if False == bInLegend:
				file.write (sLine + '\n')

			if True == bInLegend:
				if '")' in sLine:
					bInLegend = False

		file.close ()



if __name__ == '__main__':
	sys.exit (main ())

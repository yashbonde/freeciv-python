import sys, os, re
from misc import *
from config import *
from load_parse import *
from levenstein import *

lst_SegmentType = [[0,1,2],[0,2,1],[1,0,2],[1,2,0],[2,0,1],[2,1,0]]


#													
def predict_actions (_lstWords, _lstTags, _lstDeps):
	lstActions = map (lambda x: 0, _lstWords)

	print _lstWords
	print
	print _lstTags
	print
	print _lstDeps
	print
	setActions = set ()
	for i, sTag in enumerate (_lstTags):
		if True == sTag [0].startswith ('vb'):
			if i >= len (lstActions):
				continue
			lstActions [i] = 1
			setActions.add (i)

	for sType, iFrom, iTo, sFrom, sTo in _lstDeps:
		if iFrom not in setActions:
			continue
		if iTo >= len (lstActions):
			continue
		#if sType in ['nn', 'conj_or', 'dobj', 'amod', 'prep_on', 'aux']:
		if sType in ['nn', 'conj_or', 'dobj', 'amod', 'prep_on']:
			lstActions [iTo] = 1

	print 'A : ', lstActions
	return lstActions



#													
def predict_pre (_lstActions, _lstDeps):
	setPre = set ()
	lstPre = map (lambda x: 0, _lstActions)
	for sType, iFrom, iTo, sFrom, sTo in _lstDeps:
		if iFrom >= len (_lstActions):
			continue
		if 1 != _lstActions [iFrom]:
			continue
		if iTo >= len (_lstActions):
			continue
		if sType in ['nsubj', 'advcl']:
			lstPre [iTo] = 1
			setPre.add (iTo)

	for sType, iFrom, iTo, sFrom, sTo in _lstDeps:
		if iFrom not in setPre:
			continue
		if iTo >= len (_lstActions):
			continue
		#if sType in ['det', 'poss', 'nsubj', 'dobj', 'amod', 'nn', 'aux']:
		if sType in ['det', 'poss', 'nsubj', 'dobj', 'amod', 'nn']:
			lstPre [iTo] = 1

	print 'R : ', lstPre
	return lstPre
	


#													
def predict_post (_lstActions, _lstPre, _lstDeps):
	setPost = set ()
	lstPost = map (lambda x: 0, _lstActions)
	for sType, iFrom, iTo, sFrom, sTo in _lstDeps:
		if iFrom >= len (_lstActions):
			continue
		if 1 != _lstActions [iFrom]:
			continue
		if iTo >= len (_lstActions):
			continue
		if sType in ['dep', 'appos', 'xcomp']:
			lstPost [iTo] = 1
			setPost.add (iTo)

	for sType, iFrom, iTo, sFrom, sTo in _lstDeps:
		if iTo >= len (_lstActions):
			continue
		if 1 != _lstActions [iTo]:
			continue
		if sType in ['nsubj']:
			lstPost [iFrom] = 1

	for sType, iFrom, iTo, sFrom, sTo in _lstDeps:
		if iTo >= len (_lstActions):
			continue
		if 1 != _lstPre [iTo]:
			continue
		if sType in ['nsubj']:
			lstPost [iFrom] = 1

	for sType, iFrom, iTo, sFrom, sTo in _lstDeps:
		if iFrom not in setPost:
			continue
		if iTo >= len (_lstActions):
			continue
		#if sType in ['amod', 'dobj', 'num', 'nn', 'nsubj', 'conj_and', 'det', 'aux']:
		if sType in ['amod', 'dobj', 'num', 'nn', 'nsubj', 'conj_and', 'det']:
			lstPost [iTo] = 1

	print 'D : ', lstPost
	return lstPost
	


#													
def fill_blanks (_lstPred):
	sLast = None
	iLast = None
	
	iBlanks = 0
	for i, t in enumerate (_lstPred):
		if ' ' != t:
			if (iBlanks > 0) and (iBlanks < 2) and (sLast == t):
				for j in xrange (iLast + 1, i):
					_lstPred [j] = sLast
			iBlanks = 0
			iLast = i
			sLast = t
		else:
			iBlanks += 1
	
	return _lstPred



#													
def find_closest_segmentation (_lstPred):
	iLen = len (_lstPred)

	lstSeg = []
	setSeg = set ()
	for s in _lstPred:
		if '' == s.strip ():
			continue
		if s in setSeg:
			continue
		lstSeg.append (s)
		setSeg.add (s)
	sSeg = ','.join (lstSeg)

	lstBest = None
	fBest = None
	for q in xrange (6):
		if sSeg not in ','.join (map (lambda x: str(x), lst_SegmentType [q])):
			continue
		print sSeg, lst_SegmentType [q]
		for s1 in xrange (1, iLen - 1):
			for s2 in xrange (s1 + 1, iLen - 1):
				lstSeg = list (_lstPred)
				for i in xrange (iLen):
					x = 0 if (i < s1) else (1 if (i < s2) else 2)
					lstSeg [i] = str (lst_SegmentType [q][x])

				f = normalized_string_edit_distance (lstSeg, _lstPred)
				if (None == fBest) or (fBest > f):
					lstBest = list (lstSeg)
					fBest = f
	
	return lstBest



#													
def eval_assignment (_lstAnnot, _lstPred):
	iPosMatch = 0
	iPosPred = 0
	iPosActual = 0
	iNegMatch = 0
	iNegPred = 0
	iNegActual = 0
	for a, p in zip (_lstAnnot, _lstPred):
		if 1 == a:
			iPosActual += 1
		else:
			iNegActual += 1
		if 1 == p:
			iPosPred += 1
			if a == p:
				iPosMatch += 1
		else:
			iNegPred += 1
			if a == p:
				iNegMatch += 1

	return (iPosMatch, iPosPred, iPosActual, iNegMatch, iNegPred, iNegActual)



#													
def eval_acc (_lstPred, _iPredType, _lstAnnot, _sAnnotType):
	lstPred = map (lambda x: 1 if (x == _iPredType) else 0, _lstPred)
	lstAnnot = map (lambda x: 1 if (_sAnnotType in x) else 0, _lstAnnot)

	print lstAnnot
	print lstPred

	(iPosMatch, iPosPred, iPosActual, iNegMatch, iNegPred, iNegActual) = eval_assignment (lstAnnot, lstPred)
	dPosPrecis = 0
	if 0 != iPosPred:
		dPosPrecis = 1.0 * iPosMatch / iPosPred
	dPosRecall = 0
	if 0 != iPosActual:
		dPosRecall = 1.0 * iPosMatch / iPosActual
	dNegPrecis = 0
	if 0 != iNegPred:
		dNegPrecis = 1.0 * iNegMatch / iNegPred
	dNegRecall = 0
	if 0 != iNegActual:
		dNegRecall = 1.0 * iNegMatch / iNegActual

	print dPosPrecis, dNegPrecis
	return (dPosPrecis, dPosRecall, dNegPrecis, dNegRecall, 1.0 * sum(lstAnnot) / len(lstAnnot))



#													
def print_average (_sType, _lstResults):
	iCount = len (_lstResults)
	dPosPrecis = 1.0 * sum (map (lambda x: x[0], _lstResults)) / iCount
	dPosRecall = 1.0 * sum (map (lambda x: x[1], _lstResults)) / iCount
	dNegPrecis = 1.0 * sum (map (lambda x: x[2], _lstResults)) / iCount
	dNegRecall = 1.0 * sum (map (lambda x: x[3], _lstResults)) / iCount
	dRandom = 1.0 * sum (map (lambda x: x[4], _lstResults)) / iCount

	dPosF = 0
	if 0 != (dPosPrecis + dPosRecall):
		dPosF = 2.0 * dPosRecall * dPosPrecis / (dPosRecall + dPosPrecis)
	dNegF = 0
	if 0 != (dNegPrecis + dNegRecall):
		dNegF = 2.0 * dNegRecall * dNegPrecis / (dNegRecall + dNegPrecis)
	print _sType + ' : ' + ' %0.4f'%dPosRecall + '   %0.4f'%dPosPrecis + '   %0.4f'%dPosF + '   %0.4f'%dNegRecall + '   %0.4f'%dNegPrecis + '   %0.4f'%dNegF + '   %0.4f'%dRandom



#													
def main ():
	load_config (sys.argv)

	#lstText = filter (lambda x: ('' != x) and ('-----' not in x), std_read_file (get_config ('annotations')))
	lstText = filter (lambda x: ('' != x), std_read_file (get_config ('sentences')))
	lstSentenceTags = load_tags (get_config ('parses'))
	lstSentenceDeps = load_deps (get_config ('parses'))

	lstPreEven = []
	lstPostEven = []
	lstActionEven = []

	lstPreResults = []
	lstPostResults = []
	lstActionResults = []

	file = open (get_config ('prediction_output'), 'w')
	# for sText, lstTags, (lstDeps, sTree) in zip (lstText, lstSentenceTags, lstSentenceDeps):
	for sText, lstTags, (lstDeps, sTree) in zip (lstText, lstSentenceTags, lstSentenceDeps):

		lstText = sText.split ()
		lstAction = predict_actions (lstText, lstTags, lstDeps)
		print ','.join (map (lambda (x,w): w, filter (lambda (x,w): 1 == x, zip (lstAction, lstText))))
		lstPre = predict_pre (lstAction, lstDeps)
		print ','.join (map (lambda (x,w): w, filter (lambda (x,w): 1 == x, zip (lstPre, lstText))))
		lstPost = predict_post (lstAction, lstPre, lstDeps)
		print ','.join (map (lambda (x,w): w, filter (lambda (x,w): 1 == x, zip (lstPost, lstText))))

		lstPred = []
		for action, pre, post in zip (lstAction, lstPre, lstPost):
			#if 1 == action:
			#	lstPred.append ('2')
			if 1 == pre:
				lstPred.append ('0')
			elif 1 == post:
				lstPred.append ('1')
			elif 1 == action:
				lstPred.append ('2')
			else:
				lstPred.append (' ')
			if sum ([action, pre, post]) > 1:
				print 'AAAAAAAAAAAAAHHH!!'

		# lstPredFilled = fill_blanks (lstPred)
		#lstPredFilled = find_closest_segmentation (lstPred)
		lstPredFilled = lstPred

		file.write (','.join (lstPredFilled) + '\n')

	file.close ()


if __name__ == '__main__':
	sys.exit (main ())



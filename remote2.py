import wx, wx.adv, socket, pyautogui, threading, os, base64, time, sys

IP = socket.gethostbyname(socket.gethostname())
PORT = 1235
MSGSIZE = 4096
ADDRESS = (IP, PORT)
ROWS = 3
COLS = 4
TOTAL = 12
WIDTH = 1000
HEIGHT = 750
PANELWIDTH = 1000
PANELHEIGHT = 680
SIZE = 100.0
PADDING = 4
FONT = 0

pyautogui.FAILSAFE = False

class taskBarIcon(wx.adv.TaskBarIcon):
	def __init__(self, frame):
		super(taskBarIcon, self).__init__()
		
		# stores the actual app
		self.frame = frame
 
		# create dispaly tray icon
		img = wx.Image("Resources/icon.png", wx.BITMAP_TYPE_ANY)
		bmp = wx.Bitmap(img)
		self.icon = wx.Icon(bmp)
		self.SetIcon(self.icon, "toor")
		
		# actions for left and right click
		self.Bind(wx.adv.EVT_TASKBAR_LEFT_UP, self.open)
		self.Bind(wx.adv.EVT_TASKBAR_RIGHT_UP, self.showMenu)
		
		# create menu
		self.menu = self.createMenu()
 
	def createMenu(self):
		# menu has two options, restore and exit
		# may add more options in the future
		# add "Top Secret Control Panel" like discord?
		menu = wx.Menu("toor")
		menu.Append(0, "Open")
		menu.Append(1, "Exit")
		self.Bind(wx.EVT_MENU, self.open, menu.FindItemById(0))
		self.Bind(wx.EVT_MENU, self.exit, menu.FindItemById(1))
		return menu
		
	def showMenu(self, evt):
		self.PopupMenu(self.menu)
		
	def open(self, evt):
		self.frame.Show()
		self.frame.Restore()
		self.RemoveIcon()
		self.Destroy()
 
	def exit(self, evt):
		self.frame.Destroy()
		self.RemoveIcon()
		self.Destroy()
		sys.exit()

class settingsPopup(wx.Dialog):
	def __init__(self, parent, r, c, p):
		super(settingsPopup, self).__init__(parent=parent, title='Settings', size=(310, 290))
		self.SetFont(FONT)

		self.panel = wx.Panel(self)
		
		# options for customising number of rows, columns and pages
		rows = wx.StaticText(parent=self.panel, pos=(50, 43), label="Rows")
		self.rows = wx.SpinCtrl(parent=self.panel, pos=(120, 40), size=(75, 25), min=1, max=ROWS, initial=r)
		columns = wx.StaticText(parent=self.panel, pos=(50, 93), label="Columns")
		self.columns = wx.SpinCtrl(parent=self.panel, pos=(120, 90), size=(75, 25), min=1, max=COLS, initial=c)
		pages = wx.StaticText(parent=self.panel, pos=(50, 143), label="Pages")
		self.pages = wx.SpinCtrl(parent=self.panel, pos=(120, 140), size=(75, 25), min=1, initial=p)
		
		# save button
		self.save = wx.Button(self.panel, pos=(50, 210), label="Save")
		self.save.Bind(wx.EVT_BUTTON, self.update)
		
		# cancel button
		self.cancel = wx.Button(self.panel, pos=(170, 210), label="Cancel")
		self.cancel.Bind(wx.EVT_BUTTON, self.exit)
		
		self.ShowModal()
		
	def getRows(self):
		return self.rows.GetValue()
		
	def getColumns(self):
		return self.columns.GetValue()
		
	def getPages(self):
		return self.pages.GetValue()
		
	def update(self, event):
		self.GetParent().settingsUpdate(self.getRows(), self.getColumns(), self.getPages())
		self.Destroy()
		
	def exit(self, event):
		self.Destroy()

class customisePopup(wx.Dialog):
	def __init__(self, parent, pos, id, action):
		super(customisePopup, self).__init__(parent=parent, title='Customise', pos=pos, size=(400, 300))
		self.SetFont(FONT)
		
		self.panel = wx.Panel(self)
		
		# id of button
		self.id = id
		
		# options for selecting icon image
		file = wx.StaticText(parent=self.panel, pos=(50, 44), label="Icon Image")
		self.file = wx.FilePickerCtrl(parent=self.panel, pos=(120, 40), wildcard="Images (*.png)|*.png")
		self.file.SetInitialDirectory(wx.GetHomeDir())
		
		##################################################################
		# probably going to have to rewrite this cause its not very good
		
		# options for changing action type
		actionType = wx.StaticText(parent=self.panel, pos=(50, 103), label="Type")
		self.types = ["Trackpad", "Keyboard", "Numpad", "Script/File", "CMD", "Macro???"]
		self.actionType = wx.Choice(parent=self.panel, pos=(120, 100), choices=self.types)
		self.actionType.Bind(wx.EVT_CHOICE, self.changeType)
		
		# options for changing action
		act = wx.StaticText(parent=self.panel, pos=(50, 153), label="Command")
		self.action = wx.TextCtrl(parent=self.panel, pos=(120, 150), value=action)
		self.action.Disable()
		self.script = wx.FilePickerCtrl(parent=self.panel, pos=(120, 150))
		self.script.SetInitialDirectory("Scripts")
		self.script.GetTextCtrl().Destroy()
		self.script.SetTextCtrl(self.action)
		self.script.Hide()
		
		######################################################################
		
		# save button
		self.save = wx.Button(parent=self.panel, pos=(50, 220), label="Save")
		self.save.Bind(wx.EVT_BUTTON, self.update)
		
		# cancel button
		self.cancel = wx.Button(self.panel, pos=(170, 220), label="Cancel")
		self.cancel.Bind(wx.EVT_BUTTON, self.exit)
		
		self.ShowModal()
		
	def changeType(self, event):
		self.action.Disable()
		self.action.SetPosition((120, 150))
		self.script.Hide()
		if self.getActionType() in ("Trackpad", "Keyboard", "Numpad"):
			self.action.SetValue(self.getActionType().lower())
		elif self.getActionType() == "Script/File":
			self.action.Enable()
			self.script.Show()
			self.action.SetPosition((120, 151))
		elif self.getActionType() == "CMD":
			self.action.Enable()
		
	def getFile(self):
		return self.file.GetPath()
		
	def getActionType(self):
		return self.types[self.actionType.GetSelection()]
		
	def getAction(self):
		return self.action.GetLineText(1)
		
	def update(self, event):
		self.GetParent().customiseUpdate(self.id, self.getActionType(), self.getAction(), self.getFile())
		self.Destroy()
	
	def exit(self, event):
		self.Destroy()

class page(wx.Panel):
	def __init__(self, parent, pageNumber, rows, cols):
		super(page, self).__init__(parent=parent, id=pageNumber, size=(PANELWIDTH, PANELHEIGHT))
		
		self.pageNumber = pageNumber
		
		# stores icons
		self.icons = []
		
		# create ROWS * COLS grid
		for i in range(ROWS):
			for j in range(COLS):
				# load icon image
				if os.path.exists("Resources/"+str((pageNumber-1)*TOTAL+i*COLS+j)+".png"):
					img = wx.Image("Resources/"+str((pageNumber-1)*TOTAL+i*COLS+j)+".png", wx.BITMAP_TYPE_ANY)
				else:
					img = wx.Image("Resources/default.png", wx.BITMAP_TYPE_ANY)
					
				# create icon
				icon = wx.Button(parent=self, size=(SIZE+PADDING, SIZE+PADDING), id=(pageNumber-1)*TOTAL+i*COLS+j)
				icon.SetBitmap(wx.Bitmap(img))
				icon.Bind(wx.EVT_BUTTON, self.GetParent().GetParent().customise)
				
				# add icon to list
				self.icons.append(icon)
				
				# show page 1 by default
				if pageNumber != 1:
					self.Hide()
				else:
					self.Show()
		
		# set positioning
		self.updatePosition(rows, cols)
		
	def updatePosition(self, rows, cols):
		scale = min( (PANELWIDTH - 2*SIZE) / (cols * 2*SIZE - SIZE), (PANELHEIGHT - 2*SIZE) / (rows * 2*SIZE - SIZE) )
		xpadding = (PANELWIDTH - (cols * 2*SIZE - SIZE) * scale) / 2
		ypadding = (PANELHEIGHT - (rows * 2*SIZE - SIZE) * scale) / 2
		print scale
		
		for i in range(ROWS):
			for j in range(COLS):
				if i < rows and j < cols:
					self.icons[i*COLS+j].SetSize(int(SIZE*scale+PADDING), int(SIZE*scale+PADDING))
					img = self.icons[i*COLS+j].GetBitmap().ConvertToImage()
					img.Rescale(int(scale*SIZE), int(scale*SIZE))
					self.icons[i*COLS+j].SetBitmap(wx.Bitmap(img))
					self.icons[i*COLS+j].SetPosition(( int(xpadding + 2*SIZE*scale*j), int(ypadding + 2*SIZE*scale*i) ))
					self.icons[i*COLS+j].Show()
				else:
					self.icons[i*COLS+j].Hide()
	
	def updateIcon(self, id, img):
		img.Rescale(int(self.icons[id].GetWidtb()), int(self.icons[id].GetHeight()))
		self.icons[id].setBitap(wx.Bitmap(img))
	
class MainFrame(wx.Frame):    
	def __init__(self):
		super(MainFrame, self).__init__(parent=None, title='Remote', size=(WIDTH, HEIGHT))
		global FONT
		FONT = self.GetFont()
		FONT.SetPointSize(9)
		self.SetFont(FONT)
		
		# set locked sized
		self.SetSizeHints((WIDTH, HEIGHT), (WIDTH, HEIGHT))
		
		self.panel = wx.Panel(self)
		
		# set window icon
		self.SetIcon(wx.Icon("Resources/icon.png"))
		
		######################################################
		# display IP		might change to a QR code?
		ip = wx.StaticText(parent=self.panel, pos=(500, 650), label=IP)
		#####################################################
		
		# load settings/default values
		file = open("Resources/settings.dat", "r")
		self.rows, self.columns, self.numPages, self.requireSync = map(int, file.read().splitlines())
		file.close()
		file = open("Resources/updates.dat", "r")
		self.updates = list(file.read())
		file.close()
		
		# changing page controller
		self.currentPage = wx.SpinCtrl(parent=self.panel, pos=(700, 650), size=(75, 25), min=1, max=self.numPages, initial=1, style=wx.SP_ARROW_KEYS|wx.SP_WRAP)
		self.currentPage.Bind(wx.EVT_SPINCTRL, self.changePage)
		
		# settings button
		settings = wx.Button(parent=self.panel, label="Settings", pos=(800, 650))
		settings.Bind(wx.EVT_BUTTON, self.changeSettings)
		
		# load commands
		file = open("Resources/commands.dat", "r")
		self.commands = file.read().splitlines()
		file.close()
		
		# create pages
		self.pages = []
		for k in range(self.numPages):
			p = page(self.panel, k+1, self.rows, self.columns)
			self.pages.append(p)
 
		# close button
		self.Bind(wx.EVT_CLOSE, self.iconise)
		
		self.Show()
		
		# allow connections
		s = threading.Thread(target=self.connect)
		s.daemon = True
		s.start()
		
	def changePage(self, event):
		for k in range(self.numPages):
			if k+1 == self.currentPage.GetValue():
				self.pages[k].Show()
			else:
				self.pages[k].Hide()
		
	def changeSettings(self, event):
		settingsPopup(self, self.rows, self.columns, self.numPages)
		
	def settingsUpdate(self, r, c, p):
		# update rows, columns and pages
		self.rows = r
		self.columns = c
		self.numPages = p
		self.currentPage.SetMax(p)
		
		# sync if available. otherwise sync when connection made
		if self.conn:
			self.syncGrid()
		else:
			self.requireSync = 1
		
		# update settings document
		file = open("Resources/settings.dat", "w")
		file.write(str(r)+"\n")
		file.write(str(c)+"\n")
		file.write(str(p)+"\n")
		file.write(str(self.requireSync)+"\n")
		file.close()
		
		# update positioning
		for i in self.pages:
			i.updatePosition(r, c)
	
		# update pages
		if self.currentPage.GetValue() > p:
			self.currentPage.SetValue(p)
		elif p > len(self.pages):
			for k in range(len(self.pages), self.numPages):
				p = page(self.panel, k+1, r, c)
				self.pages.append(p)
			for i in range(len(self.commands), len(self.pages)*TOTAL):
				self.commands.append("")
			for i in range(len(self.updates), len(self.pages)*TOTAL):
				self.updates.append("0")
				
		self.changePage(None)
	
	def customise(self, event):
		btn = event.GetEventObject()
		customisePopup(self, btn.GetScreenPosition(), btn.GetId(), self.commands[btn.GetId()])
		
	def customiseUpdate(self, id, actionType, action, filename):
		#############################################################
		# rewrite probably
		if self.commands[id] != action:
			if actionType == "Script/File":
				self.commands[id] = "start "+action
			else:
				self.commands[id] = action
			file = open("Resources/commands.dat", "w")
			for i in self.commands:
				file.write(i+"\n")
			file.close()
		###########################################################
		
		if filename:
			img = wx.Image(filename, wx.BITMAP_TYPE_ANY)
			s = 100.0/max(img.GetWidth(), img.GetHeight())
			img.Rescale(int(img.GetWidth()*s), int(img.GetHeight()*s))
			img.Resize(size=(SIZE, SIZE), pos=((SIZE-img.GetWidth())/2, (SIZE-img.GetHeight())/2));
			img.SaveFile("Resources/"+str(id)+".png", wx.BITMAP_TYPE_PNG)
			self.pages[id%TOTAL].updateIcon(id/TOTAL, img)
			
			if self.conn:
				self.syncImage(id)
			else:
				self.updates[id] = "1"
				file = open("Resources/updates.dat", "r+")
				file.seek(id)
				file.write("1")
				file.close()
			
	def syncGrid(self):
		print("SYNC GRID")
		self.send("grid")
		time.sleep(1)
		self.send(str(self.numPages))
		time.sleep(1)
		self.send(str(self.rows))
		time.sleep(1)
		self.send(str(self.columns))
		print("DONE")
		
	def syncImage(self, id):
		print("SYNC IMAGE")
		self.send("image")
		time.sleep(1)
		self.send(str(id))
		time.sleep(1)
		file = open("Resources/"+str(id)+".png", "rb")
		lines = base64.b64encode(file.read())
		file.close()
		self.send(lines)
		print("DONE")
			
	def connect(self):
		# initiate server
		server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		server.bind(ADDRESS)
		server.listen(1)
		
		# server loop
		while True:
			self.conn = None
			print("RESET")
			self.conn, address = server.accept()
			c = True
			print("accepted")
			
			# check if grid requires syncing
			if self.requireSync:
				self.syncGrid(True)
				self.requireSync = False
				file = open("Resources/settings.dat", "w")
				file.write(str(r)+"\n")
				file.write(str(c)+"\n")
				file.write(str(p)+"\n")
				file.write(str(self.requireSync)+"\n")
				file.close()
				
			# check if icons require syncing
			file = open("Resources/updates.dat", "r+")
			for i in range(self.updates):
				if self.updates[i] == "1":
					self.syncImage(i)
					self.updates[i] = "0"
					file.seek(id)
					file.write("0")
			file.close()

			while c:
				data = self.conn.recv(MSGSIZE)
				c = self.get(data)
			
	def get(self, data):
		# tries to perform action
		# if an exception occurs terminate connection
		print("DATA:", data, "DATA END")
		try:
			self.run(int(data))
			return True
		except:
			return False
			
	def run(self, id):
		# runs command
		c = self.commands[id]
		if c in ("trackpad", "keyboard", "numpad"):
			# requires client to change screen
			self.send(c)
			receiver(c)
		else:
			os.system(c)
			
	def send(self, msg):
		self.conn.send(msg+"EOFEOFEOFEOFEOFEOFEOFEOFXXX")
	
	def iconise(self, event):
		# shrink to display tray icon
		self.Iconize()
		taskBarIcon(self)
		self.Hide()
			
def receiver(utility):
	# handles utilities (trackpad, keyboard, numpad)

	server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	server.bind(ADDRESS)
	print(utility)

	while True:
		data, address = server.recvfrom(4096)
		if data == "exit":
			return
		if utility == "trackpad":
			threading.Thread(target=trackpad, args=(data,)).start()
		elif utility == "keyboard":
			threading.Thread(target=keyboard, args=(data,)).start()
		elif utility == "numpad":
			threading.Thread(target=numpad, args=(data,)).start()

def trackpad(data):
	print(data)
	
	if data.startswith("move"):
		x, y = map(float, data[5:].split())
		pyautogui.move(x, y)
		
	elif data == "drag start":
		pyautogui.mouseDown()
	elif data == "drag end":
		pyautogui.mouseUp()
			
	elif data == "left click":
		pyautogui.click()
	elif data == "right click":
		pyautogui.rightClick()
		
	elif data.startswith("scroll"):
		delta = int(data[7:])
		pyautogui.scroll(delta)
		
	elif data == "zoom in":
		pyautogui.hotkey("ctrl", "+")
	elif data == "zoom out":
		pyautogui.hotkey("ctrl", "-")
			
	elif data == "search":
		pyautogui.hotkey("win", "s")
	elif data == "show all":
		pyautogui.hotkey("win", "tab")
	elif data == "show desktop":
		pyautogui.hotkey("win", "d")
	elif data == "show next":
		pyautogui.hotkey("ctrl", "alt", "tab")
	elif data == "show prev":
		pyautogui.hotkey("ctrl", "alt", "shift", "tab")
	elif data == "show end":
		pyautogui.press("enter")
		
	elif data == "desk next":
		pyautogui.hotkey("win", "ctrl", "right")
	elif data == "desk prev":
		pyautogui.hotkey("win", "ctrl", "left")

def keyboard(data):
	print(data)
	pyautogui.press(data)
	
def numpad(data):
	print(data)
	pyautogui.press(data)


def main():
	app = wx.App()
	frame = MainFrame()
	app.MainLoop()
			

if __name__ == '__main__':
	main()
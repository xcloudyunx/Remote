import wx, wx.adv, socket, pyautogui, threading, os, base64, time, sys, pyqrcode, keyboard

IP = socket.gethostbyname(socket.gethostname())
PORT = 1235
MSGSIZE = 4096
ADDRESS = (IP, PORT)
ROWS = 3
COLS = 4
TOTAL = 12
SIZE = 100
PANELWIDTH = 40+(COLS-1)*5+COLS*SIZE
PANELHEIGHT = 40+(ROWS-1)*5+ROWS*SIZE

pyautogui.FAILSAFE = False

class taskBarIcon(wx.adv.TaskBarIcon):
	def __init__(self, frame):
		super().__init__()
		
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

class connectionPopup(wx.Dialog):
	def __init__(self, parent):
		super().__init__(parent=parent, title='New Connection')

		self.panel = wx.Panel(self)
		
		# generate IP and display
		qrcode = pyqrcode.create(IP)
		qrcode.png("Resources/IP.png", scale=10)
		ip = wx.Image("Resources/IP.png", wx.BITMAP_TYPE_ANY).ConvertToBitmap()
		ip = wx.StaticBitmap(parent=self.panel, bitmap=ip)
		
		# exit button
		self.exitButton = wx.Button(parent=self.panel, label="Exit")
		self.exitButton.Bind(wx.EVT_BUTTON, self.exit)
		
		# positioning
		topSizer = wx.BoxSizer(wx.VERTICAL)
		
		topSizer.Add(ip, flag=wx.ALL|wx.CENTER, border=5)
		topSizer.Add(wx.StaticLine(self.panel), flag=wx.ALL|wx.EXPAND, border=5)
		topSizer.Add(self.exitButton, flag=wx.ALL|wx.CENTER, border=5)
		
		self.panel.SetSizer(topSizer)
		topSizer.Fit(self)
		
		self.ShowModal()
		
	def exit(self, event):
		self.Destroy()

class settingsPopup(wx.Dialog):
	def __init__(self, parent, r, c, p):
		super().__init__(parent=parent, title='Settings')

		self.panel = wx.Panel(self)
		
		# options for customising number of rows, columns and pages
		rows = wx.StaticText(parent=self.panel, label="Rows")
		self.rows = wx.SpinCtrl(parent=self.panel, min=1, max=ROWS, initial=r)
		columns = wx.StaticText(parent=self.panel, label="Columns")
		self.columns = wx.SpinCtrl(parent=self.panel, min=1, max=COLS, initial=c)
		pages = wx.StaticText(parent=self.panel, label="Pages")
		self.pages = wx.SpinCtrl(parent=self.panel, min=1, initial=p)
		
		# save button
		self.save = wx.Button(self.panel, label="Save")
		self.save.Bind(wx.EVT_BUTTON, self.update)
		
		# cancel button
		self.cancel = wx.Button(self.panel, label="Cancel")
		self.cancel.Bind(wx.EVT_BUTTON, self.exit)
		
		
		# positioning
		topSizer = wx.BoxSizer(wx.VERTICAL)
		gridSizer = wx.FlexGridSizer(rows=3, cols=2, vgap=20, hgap=20)
		gridSizer.AddGrowableCol(1)
		buttonSizer = wx.BoxSizer(wx.HORIZONTAL)
		
		gridSizer.Add(rows)
		gridSizer.Add(self.rows, flag=wx.EXPAND)
		gridSizer.Add(columns)
		gridSizer.Add(self.columns, flag=wx.EXPAND)
		gridSizer.Add(pages)
		gridSizer.Add(self.pages, flag=wx.EXPAND)
		
		buttonSizer.Add(self.save, flag=wx.ALL, border=5)
		buttonSizer.Add(self.cancel, flag=wx.ALL, border=5)
		
		topSizer.Add(gridSizer, flag=wx.ALL|wx.EXPAND, border=20)
		topSizer.Add(wx.StaticLine(self.panel), flag=wx.ALL|wx.EXPAND, border=5)
		topSizer.Add(buttonSizer, flag=wx.ALL|wx.CENTER, border=5)
		
		self.panel.SetSizer(topSizer)
		topSizer.Fit(self)
		
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
		super().__init__(parent=parent, title='Customise', pos=pos)
		
		self.panel = wx.Panel(self)
		
		# id of button
		self.id = id
		
		# options for selecting icon image
		file = wx.StaticText(parent=self.panel, label="Icon Image")
		self.file = wx.FilePickerCtrl(parent=self.panel, wildcard="Images (*.png)|*.png")
		self.file.SetInitialDirectory(wx.GetHomeDir())
		
		self.clear = wx.Button(parent=self.panel, label="Clear Image")
		self.clear.Bind(wx.EVT_BUTTON, self.clearImage)
		
		##################################################################
		# probably going to have to rewrite this cause its not very good
		
		# options for changing action type
		actionType = wx.StaticText(parent=self.panel, label="Type")
		self.types = ["Trackpad", "Keyboard", "Numpad", "Script/File", "CMD", "Macro", "Multimedia"]
		self.actionType = wx.Choice(parent=self.panel, choices=self.types)
		self.actionType.Bind(wx.EVT_CHOICE, self.changeType)
		
		# options for changing action
		act = wx.StaticText(parent=self.panel, label="Command")
		self.action = wx.TextCtrl(parent=self.panel, value=action)
		self.action.Disable()
		
		self.m = ["macro", [], 1.0]
		self.rm = wx.Button(parent=self.panel, label="Record")
		self.rm.Bind(wx.EVT_BUTTON, self.recordMacro)
		self.rm.Hide()
		self.sm = wx.Button(parent=self.panel, label="Stop")
		self.sm.Bind(wx.EVT_BUTTON, self.stopMacro)
		self.sm.Hide()
		
		self.media = ["Play/Pause", "Stop", "Previous", "Next", "Volume Down", "Volume Up", "Mute Toggle"]
		self.med = ["playpause", "stop", "prevtrack", "nexttrack", "volumedown", "volumeup", "volumemute"]
		self.multimedia = wx.Choice(parent=self.panel, choices=self.media)
		self.multimedia.Hide()
		
		self.script = wx.FilePickerCtrl(parent=self.panel)
		self.script.SetInitialDirectory("Scripts")
		self.script.Hide()
		
		######################################################################
		
		# save button
		self.save = wx.Button(parent=self.panel, label="Save")
		self.save.Bind(wx.EVT_BUTTON, self.update)
		
		# cancel button
		self.cancel = wx.Button(self.panel, label="Cancel")
		self.cancel.Bind(wx.EVT_BUTTON, self.exit)
		
		# positioning
		topSizer = wx.BoxSizer(wx.VERTICAL)
		gridSizer = wx.FlexGridSizer(rows=3, cols=2, vgap=20, hgap=20)
		gridSizer.AddGrowableCol(1)
		sizerOne = wx.BoxSizer(wx.HORIZONTAL)
		sizerTwo = wx.BoxSizer(wx.HORIZONTAL)
		buttonSizer = wx.BoxSizer(wx.HORIZONTAL)
		
		gridSizer.Add(file)
		sizerOne.Add(self.file, 1, flag=wx.RIGHT|wx.EXPAND, border=5)
		sizerOne.Add(self.clear)
		gridSizer.Add(sizerOne, flag=wx.EXPAND)
		gridSizer.Add(actionType)
		gridSizer.Add(self.actionType, flag=wx.EXPAND)
		gridSizer.Add(act)
		sizerTwo.Add(self.action, 1, flag=wx.EXPAND)
		sizerTwo.Add(self.rm, flag=wx.LEFT, border=5)
		sizerTwo.Add(self.sm, flag=wx.LEFT, border=5)
		sizerTwo.Add(self.multimedia, 1, flag=wx.EXPAND)
		sizerTwo.Add(self.script, 1, flag=wx.EXPAND)
		gridSizer.Add(sizerTwo, flag=wx.EXPAND)
		
		buttonSizer.Add(self.save, flag=wx.ALL, border=5)
		buttonSizer.Add(self.cancel, flag=wx.ALL, border=5)
		
		topSizer.Add(gridSizer, flag=wx.ALL|wx.EXPAND, border=20)
		topSizer.Add(wx.StaticLine(self.panel), flag=wx.ALL|wx.EXPAND, border=5)
		topSizer.Add(buttonSizer, flag=wx.ALL|wx.CENTER, border=5)
		
		self.panel.SetSizer(topSizer)
		topSizer.Fit(self)
		
		self.ShowModal()

	def clearImage(self, event):
		self.file.SetPath("RESET")
	
	def changeType(self, event):
		self.action.Disable()
		self.action.Hide()
		self.rm.Hide()
		if self.sm.IsShown():
			keyboard.stop_recording()
			self.sm.Hide()
		self.multimedia.Hide()
		self.script.Hide()
		if self.getActionType() in ("Trackpad", "Keyboard", "Numpad"):
			self.action.SetValue(self.getActionType().lower())
			self.action.Show()
		elif self.getActionType() == "Script/File":
			self.script.Show()
		elif self.getActionType() == "CMD":
			self.action.Enable()
			self.action.Show()
		elif self.getActionType() == "Macro":
			self.action.Show()
			self.rm.Show()
		elif self.getActionType() == "Multimedia":
			self.multimedia.Show()
		self.panel.GetSizer().Layout()
			
	def recordMacro(self, event):
		self.rm.Hide()
		self.sm.Show()
		self.panel.GetSizer().Layout()
		keyboard.start_recording()
		
	def stopMacro(self, event):
		self.m[1] = keyboard.stop_recording()
		self.action.SetValue(str(self.m))
		self.sm.Hide()
		self.rm.Show()
		self.panel.GetSizer().Layout()
		
	def getFile(self):
		return self.file.GetPath()
		
	def getActionType(self):
		return self.types[self.actionType.GetSelection()]
		
	def getAction(self):
		if self.getActionType() == "Script/File":
			return self.script.GetTextCtrl.GetLineText(0)
		elif self.getActionType() == "Macro":
			return self.m
		elif self.getActionType() == "Multimedia":
			return ["multimedia", self.med[self.multimedia.GetSelection()]]
		return self.action.GetLineText(0)
		
	def update(self, event):
		self.GetParent().customiseUpdate(self.id, self.getActionType(), self.getAction(), self.getFile())
		self.Destroy()
	
	def exit(self, event):
		self.Destroy()

class page(wx.Panel):
	def __init__(self, parent, pageNumber, rows, cols):
		super().__init__(parent=parent, id=pageNumber, size=(PANELWIDTH, PANELHEIGHT))
		
		self.pageNumber = pageNumber
		
		# stores icons
		self.icons = []
		
		# positioning
		topSizer = wx.BoxSizer(wx.VERTICAL)
		self.sizers = [wx.BoxSizer(wx.HORIZONTAL) for i in range(ROWS)]
		
		# create ROWS * COLS grid
		for i in range(ROWS):
			for j in range(COLS):
				# create icon
				icon = wx.Button(parent=self, size=(1,1), id=(pageNumber-1)*TOTAL+i*COLS+j)
				
				# add icon to row sizer
				self.sizers[i].Add(icon, 1, flag=wx.SHAPED|wx.ALIGN_CENTRE)
			
				# load icon image
				if os.path.exists("Resources/"+str((pageNumber-1)*TOTAL+i*COLS+j)+".png"):
					img = wx.Image("Resources/"+str((pageNumber-1)*TOTAL+i*COLS+j)+".png", wx.BITMAP_TYPE_ANY)
				else:
					img = wx.Image("Resources/default.png", wx.BITMAP_TYPE_ANY)
				icon.SetBitmap(wx.Bitmap(img))
				
				icon.Bind(wx.EVT_BUTTON, self.GetParent().GetParent().customise)
				
				# add icon to list
				self.icons.append(icon)
				
				# show page 1 by default
				if pageNumber != 1:
					self.Hide()
				else:
					self.Show()
			
			# add row sizer to main sizer
			topSizer.Add(self.sizers[i], 1, flag=wx.SHAPED|wx.ALIGN_CENTRE)
		
		self.SetSizer(topSizer)
		topSizer.Fit(self)
		
		# set positioning
		self.updatePosition(rows, cols)
		
	def showPage(self, rows, cols):
		self.Show()
		self.updatePosition(rows, cols)
		
	def updatePosition(self, rows, cols):
		for i in range(ROWS):
			for j in range(COLS):
				if i < rows and j < cols:
					bmp = self.icons[i*COLS+j].GetBitmap()
					img = bmp.ConvertToImage()
					self.icons[i*COLS+j].SetBitmap(wx.Bitmap(img))
					self.icons[i*COLS+j].Show()
				else:
					self.icons[i*COLS+j].Hide()
					
		self.GetSizer().Layout()
	
	def updateIcon(self, id, img):
		if img == "RESET":
			img = wx.Image("Resources/default.png", wx.BITMAP_TYPE_ANY)
		img.Rescale(int(self.icons[id].GetBitmap().GetWidth()), int(self.icons[id].GetBitmap().GetHeight()))
		self.icons[id].SetBitmap(wx.Bitmap(img))
	
class MainFrame(wx.Frame):    
	def __init__(self):
		super().__init__(parent=None, title='Remote')
		
		self.panel = wx.Panel(self)
		
		# set window icon
		self.SetIcon(wx.Icon("Resources/icon.png"))
		
		#################################################
		# new connection button
		#newConn = wx.Button(parent=self.panel, label="New Connection")
		#newConn.Bind(wx.EVT_BUTTON, self.newConnection)
		#####################################################
		newConn = wx.StaticText(parent=self.panel, label=IP)
		
		# load settings/default values
		file = open("Resources/settings.dat", "r")
		self.rows, self.columns, self.numPages, self.requireSync = map(int, file.read().splitlines())
		file.close()
		file = open("Resources/updates.dat", "r")
		self.updates = list(file.read())
		for i in range(len(self.updates), self.numPages*TOTAL):
			self.updates.append(0)
		file.close()
		
		# changing page controller
		self.currentPage = wx.SpinCtrl(parent=self.panel, min=1, max=self.numPages, initial=1, style=wx.SP_ARROW_KEYS|wx.SP_WRAP)
		self.currentPage.Bind(wx.EVT_SPINCTRL, self.changePage)
		
		# settings button
		settings = wx.Button(parent=self.panel, label="Settings")
		settings.Bind(wx.EVT_BUTTON, self.changeSettings)
		
		topSizer = wx.BoxSizer(wx.VERTICAL)
		sizerTwo = wx.BoxSizer(wx.HORIZONTAL)
		
		# create pages
		self.pages = []
		for k in range(self.numPages):
			p = page(self.panel, k+1, self.rows, self.columns)
			self.pages.append(p)
			sizerTwo.Add(p, flag=wx.EXPAND|wx.ALL, border=20)
			
			
		# positioning
		sizerOne = wx.BoxSizer(wx.HORIZONTAL)
		
		sizerOne.Add(settings, flag=wx.ALL|wx.ALIGN_CENTRE, border=20)
		sizerOne.Add(self.currentPage, 1, flag=wx.EXPAND|wx.ALL, border=20)
		sizerOne.Add(newConn, flag=wx.ALL|wx.ALIGN_CENTRE, border=20)
		
		topSizer.Add(sizerTwo, flag=wx.EXPAND)
		topSizer.Add(sizerOne, flag=wx.EXPAND)
		
		self.panel.SetSizer(topSizer)
		topSizer.Fit(self)
			
		# load commands
		file = open("Resources/commands.dat", "r")
		self.commands = file.read().splitlines()
		for i in range(len(self.commands)):
			if self.commands[i].startswith("["):
				self.commands[i] = eval(self.commands[i])
		file.close()
		for i in range(len(self.commands), len(self.pages)*ROWS*COLS):
			self.commands.append("")
			
		# close button
		self.Bind(wx.EVT_CLOSE, self.iconise)
		
		self.Show()
		
		# allow connections
		threading.Thread(target=self.connect, daemon=True).start()
		
	def changePage(self, event):
		for k in range(self.numPages):
			if k+1 == self.currentPage.GetValue():
				self.pages[k].showPage(self.rows, self.columns)
			else:
				self.pages[k].Hide()
		self.panel.GetSizer().Layout()
				
	def newConnection(self, event):
		connectionPopup(self)
		
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
		customisePopup(self, btn.GetScreenPosition(), btn.GetId(), str(self.commands[btn.GetId()]))
		
	def customiseUpdate(self, id, actionType, action, filename):
		#############################################################
		# rewrite probably
		if self.commands[id] != action:
			if actionType == "Script/File":
				self.commands[id] = '"'+action+'"'
			else:
				self.commands[id] = action
			file = open("Resources/commands.dat", "w")
			for i in range(min(self.numPages*TOTAL, len(self.commands))):
				file.write(str(self.commands[i])+"\n")
			file.close()
		###########################################################
		
		if filename:
			img = ""
			if filename == "RESET":
				if os.path.exists("Resources/"+str(id)+".png"):
					os.remove("Resources/"+str(id)+".png")
				img = "RESET"
			else:
				img = wx.Image(filename, wx.BITMAP_TYPE_ANY)
				s = 100.0/max(img.GetWidth(), img.GetHeight())
				img.Rescale(int(img.GetWidth()*s), int(img.GetHeight()*s))
				img.Resize(size=(SIZE, SIZE), pos=((SIZE-img.GetWidth())//2, (SIZE-img.GetHeight())//2));
				img.SaveFile("Resources/"+str(id)+".png", wx.BITMAP_TYPE_PNG)
			print(id%TOTAL)
			self.pages[id//TOTAL].updateIcon(id%TOTAL, img)
			
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
		if os.path.exists("Resources/"+str(id)+".png"):
			self.send("new image")
			time.sleep(1)
			self.send(str(id))
			file = open("Resources/"+str(id)+".png", "rb")
			lines = base64.b64encode(file.read()).decode()
			file.close()
			self.send(lines)
		else:
			self.send("reset image")
			time.sleep(1)
			self.send(str(id))
			
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
				self.syncGrid()
				self.requireSync = 0
				file = open("Resources/settings.dat", "w")
				file.write(str(self.rows)+"\n")
				file.write(str(self.columns)+"\n")
				file.write(str(self.numPages)+"\n")
				file.write(str(self.requireSync)+"\n")
				file.close()
				
			# check if icons require syncing
			file = open("Resources/updates.dat", "r+")
			for i in range(len(self.updates)):
				if self.updates[i] == "1":
					self.syncImage(i)
					self.updates[i] = "0"
					file.seek(i)
					file.write("0")
			file.close()

			while c:
				data = self.conn.recv(MSGSIZE).decode()
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
		elif c[0] == "macro":
			keyboard.play(c[1], speed_factor=c[2])
		elif c[0] == "multimedia":
			pyautogui.press(c[1])
		else:
			os.system(c)
			
	def send(self, msg):
		self.conn.send((msg+"EOFEOFEOFEOFEOFEOFEOFEOFXXX").encode())
	
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
		data = data.decode()
		if data == "exit":
			print("exit", utility)
			return
		if utility == "trackpad":
			threading.Thread(target=trackpadFunction, args=(data,)).start()
		elif utility == "keyboard":
			threading.Thread(target=keyboardFunction, args=(data,)).start()
		elif utility == "numpad":
			threading.Thread(target=numpadFunction, args=(data,)).start()

def trackpadFunction(data):
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
	elif data == "show start":
		pyautogui.keyDown("alt")
	elif data == "show next":
		pyautogui.hotkey("tab")
	elif data == "show prev":
		pyautogui.hotkey("shift", "tab")
	elif data == "show end":
		pyautogui.keyUp("alt")
	
	elif data == "action centre":
		pyautogui.hotkey("win", "a")
	elif data == "desk next":
		pyautogui.hotkey("win", "ctrl", "right")
	elif data == "desk prev":
		pyautogui.hotkey("win", "ctrl", "left")

def keyboardFunction(data):
	print(data)
	pyautogui.press(data)
	
def numpadFunction(data):
	print(data)
	pyautogui.press(data)


def main():
	app = wx.App()
	frame = MainFrame()
	app.MainLoop()
			

if __name__ == '__main__':
	main()
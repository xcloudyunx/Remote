import wx, wx.adv, socket, pyautogui, threading, os, base64, time

IP = socket.gethostbyname(socket.gethostname())
PORT = 1235
SIZE = 4096
ADDRESS = (IP, PORT)
ROWS = 3
COLS = 4
TOTAL = 12

pyautogui.FAILSAFE = False

class CustomTaskBarIcon(wx.adv.TaskBarIcon):
	def __init__(self, frame):
		super(CustomTaskBarIcon, self).__init__()
		self.frame = frame
 
		img = wx.Image("Resources/icon.png", wx.BITMAP_TYPE_ANY)
		bmp = wx.Bitmap(img)
		self.icon = wx.Icon()
		self.icon.CopyFromBitmap(bmp)
 
		self.SetIcon(self.icon, "Remote")
		self.Bind(wx.adv.EVT_TASKBAR_LEFT_UP, self.OnTaskBarLeftClick)
		self.Bind(wx.adv.EVT_TASKBAR_RIGHT_UP, self.OnTaskBarRightClick)
		
		self.menu = self.CreatePopupMenu()
 
	def OnTaskBarActivate(self, evt):
		pass
 
	def OnTaskBarClose(self, evt):
		self.frame.Close()

	def OnTaskBarLeftClick(self, evt):
		self.frame.Show()
		self.frame.Restore()
		self.RemoveIcon()
		self.Destroy()
		
	def OnTaskBarRightClick(self, evt):
		self.PopupMenu(self.menu)
		
	def CreatePopupMenu(self):
		menu = wx.Menu("test")
		menu.Append(5, "hello")
		return menu

class settingsPopup(wx.Dialog):
	def __init__(self, parent, r, c, p):
		super(settingsPopup, self).__init__(parent=parent, title='Settings', size=(400, 350))
		
		self.panel = wx.Panel(self)
		
		file = wx.StaticText(parent=self.panel, pos=(50, 44), label="Background")
		self.file = wx.FilePickerCtrl(parent=self.panel, pos=(120, 40), wildcard="Images (*.png)|*.png")
		self.file.SetInitialDirectory(wx.GetHomeDir())
		
		rows = wx.StaticText(parent=self.panel, pos=(50, 103), label="Rows")
		self.rows = wx.SpinCtrl(parent=self.panel, pos=(120, 100), size=(75, 25), min=1, max=ROWS, initial=r)
		columns = wx.StaticText(parent=self.panel, pos=(50, 153), label="Columns")
		self.columns = wx.SpinCtrl(parent=self.panel, pos=(120, 150), size=(75, 25), min=1, max=COLS, initial=c)
		pages = wx.StaticText(parent=self.panel, pos=(50, 203), label="Pages")
		self.pages = wx.SpinCtrl(parent=self.panel, pos=(120, 200), size=(75, 25), min=1, initial=p)
		
		self.save = wx.Button(self.panel, pos=(50, 270), label="Save")
		self.save.Bind(wx.EVT_BUTTON, self.update)
		
		self.ShowModal()
		
	def update(self, event):
		self.GetParent().settingsUpdate(self.getFile(), self.getRows(), self.getColumns(), self.getPages())
		self.Destroy()
		
	def getRows(self):
		return self.rows.GetValue()
		
	def getColumns(self):
		return self.columns.GetValue()
		
	def getPages(self):
		return self.pages.GetValue()
		
	def getFile(self):
		return self.file.GetPath()

class customisePopup(wx.Dialog):
	def __init__(self, parent, pos, id, action):
		super(customisePopup, self).__init__(parent=parent, title='Customise', pos=pos, size=(400, 300))
		
		self.panel = wx.Panel(self)
		
		file = wx.StaticText(parent=self.panel, pos=(50, 44), label="Icon Image")
		self.file = wx.FilePickerCtrl(parent=self.panel, pos=(120, 40), wildcard="Images (*.png)|*.png")
		self.file.SetInitialDirectory(wx.GetHomeDir())
		
		actionType = wx.StaticText(parent=self.panel, pos=(50, 103), label="Type")
		self.types = ["Trackpad", "Keyboard", "Numpad", "Script/File", "CMD", "Macro???"]
		self.actionType = wx.Choice(parent=self.panel, pos=(120, 100), choices=self.types)
		self.actionType.Bind(wx.EVT_CHOICE, self.changeType)
		
		act = wx.StaticText(parent=self.panel, pos=(50, 153), label="Command")
		self.action = wx.TextCtrl(parent=self.panel, pos=(120, 150), value=action)
		self.action.Disable()
		self.script = wx.FilePickerCtrl(parent=self.panel, pos=(120, 150))
		self.script.SetInitialDirectory("Scripts")
		self.script.GetTextCtrl().Destroy()
		self.script.SetTextCtrl(self.action)
		self.script.Hide()
		
		self.save = wx.Button(parent=self.panel, pos=(50, 220), label="Save")
		self.save.Bind(wx.EVT_BUTTON, self.update)
		
		self.id = id
		
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
		
	def update(self, event):
		self.GetParent().customiseUpdate(self.id, self.getActionType(), self.getAction(), self.getFile())
		self.Destroy()
		
	def getFile(self):
		return self.file.GetPath()
		
	def getActionType(self):
		return self.types[self.actionType.GetSelection()]
		
	def getAction(self):
		return self.action.GetLineText(1)	

class MainFrame(wx.Frame):    
	def __init__(self):
		super(MainFrame, self).__init__(parent=None, title='Remote', size=(1000, 750))
		
		self.SetSizeHints((1000, 750), (1000, 750))
		
		self.panel = wx.Panel(self)
		self.background = wx.StaticBitmap(parent=self.panel, bitmap=wx.Bitmap("Resources/background.png"))
		
		self.SetIcon(wx.Icon("Resources/icon.png"))
		
		file = open("Resources/settings.txt", "r")
		self.rows, self.columns, self.pages = map(int, file.read().splitlines())
		
		self.currentPage = wx.SpinCtrl(parent=self.background, pos=(700, 650), size=(75, 25), min=1, max=self.pages, initial=1, style=wx.SP_ARROW_KEYS|wx.SP_WRAP)
		self.currentPage.Bind(wx.EVT_SPINCTRL, self.changePage)
		
		settings = wx.Button(parent=self.background, label="Settings", pos=(800, 650))
		settings.Bind(wx.EVT_BUTTON, self.changeSettings)
		
		ip = wx.StaticText(parent=self.background, pos=(500, 650), label=IP)
		
		file = open("Resources/commands.txt", "r")
		self.commands = file.read().splitlines()
		file.close()
			
		self.icons = []
		for k in xrange(self.pages):
			for i in xrange(ROWS):
				for j in xrange(COLS):
					if os.path.exists("Resources/"+str(k*TOTAL+i*COLS+j)+".png"):
						img = wx.Image("Resources/"+str(k*TOTAL+i*COLS+j)+".png", wx.BITMAP_TYPE_ANY)
					else:
						img = wx.Image("Resources/default.png", wx.BITMAP_TYPE_ANY)
					s = 100.0/max(img.GetWidth(), img.GetHeight())
					img.Rescale(img.GetWidth()*s, img.GetHeight()*s)
					icon = wx.Button(parent=self.background, pos=(145+200*j, 100+200*i), size=(110, 110), id=k*TOTAL+i*COLS+j)
					icon.SetBitmap(wx.Bitmap(img))
					icon.Bind(wx.EVT_BUTTON, self.customise)
					if k > 0 or i >= self.rows or j >= self.columns:
						icon.Hide()
					self.icons.append(icon)
 
		self.Bind(wx.EVT_CLOSE, self.onClose)
		
		self.Show()
		
		threading.Thread(target=self.connect).start()
		
	def changePage(self, event):
		for k in xrange(self.pages):
			for i in xrange(ROWS):
				for j in xrange(COLS):
					if k+1 == self.currentPage.GetValue() and i < self.rows and j < self.columns:
						self.icons[k*TOTAL+i*COLS+j].Show()
					else:
						self.icons[k*TOTAL+i*COLS+j].Hide()
		
	def changeSettings(self, event):
		self.box = settingsPopup(self, self.rows, self.columns, self.pages)
		
	def settingsUpdate(self, filename, r, c, p):
		if filename:
			img = wx.Image(filename, wx.BITMAP_TYPE_ANY)
			s = max(1000/img.GetWidth(), 750/img.GetHeight())
			img.Rescale(img.GetWidth()*s, img.GetHeight()*s)
			img.SaveFile("Resources/background.png", wx.BITMAP_TYPE_PNG)
	
		self.rows = r
		self.columns = c
		self.pages = p
		self.currentPage.SetMax(p)
		
		file = open("Resources/settings.txt", "w")
		file.write(str(r)+"\n")
		file.write(str(c)+"\n")
		file.write(str(p)+"\n")
		file.close()
	
		if self.currentPage.GetValue() > p:
			self.currentPage.SetValue(p)
		elif p > len(self.icons)/TOTAL:
			for k in xrange(len(self.icons)/TOTAL, p):
				for i in xrange(ROWS):
					for j in xrange(COLS):
						if os.path.exists("Resources/"+str(k*TOTAL+i*COLS+j)+".png"):
							img = wx.Image("Resources/"+str(k*TOTAL+i*COLS+j)+".png", wx.BITMAP_TYPE_ANY)
						else:
							img = wx.Image("Resources/default.png", wx.BITMAP_TYPE_ANY)
						s = 100.0/max(img.GetWidth(), img.GetHeight())
						img.Rescale(img.GetWidth()*s, img.GetHeight()*s)
						icon = wx.Button(parent=self.background, pos=(145+200*j, 100+200*i), size=(110, 110), id=k*TOTAL+i*COLS+j)
						icon.SetBitmap(wx.Bitmap(img))
						icon.Bind(wx.EVT_BUTTON, self.customise)
						icon.Hide()
						self.icons.append(icon)
			for i in xrange(len(self.commands), len(self.icons)):
				self.commands.append("")
		
		self.changePage(None)
		
		self.syncGrid(filename)
	
	def customise(self, event):
		btn = event.GetEventObject()
		self.box = customisePopup(self, self.GetPosition()+btn.GetPosition(), btn.GetId(), self.commands[btn.GetId()])
		self.box.Position = btn.GetScreenPosition()
		
	def customiseUpdate(self, id, actionType, action, filename):
		if self.commands[id] != action:
			if actionType == "Script/File":
				self.commands[id] = "start "+action
			else:
				self.commands[id] = action
			file = open("Resources/commands.txt", "w")
			for i in self.commands:
				file.write(i+"\n")
			file.close()
		if filename:
			img = wx.Image(filename, wx.BITMAP_TYPE_ANY)
			s = 100.0/max(img.GetWidth(), img.GetHeight())
			img.Rescale(img.GetWidth()*s, img.GetHeight()*s)
			img.Resize(size=(100, 100), pos=((100-img.GetWidth())/2, (100-img.GetHeight())/2));
			img.SaveFile("Resources/"+str(id)+".png", wx.BITMAP_TYPE_PNG)
			self.icons[id].SetBitmap(wx.Bitmap(img))
			self.syncImage(id)
		
	def run(self, id):
		c = self.commands[id]
		if c in ("trackpad", "keyboard", "numpad"):
			self.send(c)
			receiver(c)
		else:
			os.system(c)
		
	def syncGrid(self, filename):
		print "SYNC GRID"
		if filename:
			self.send("background")
			time.sleep(1)
			file = open("Resources/background.png", "rb")
			lines = base64.b64encode(file.read())
			file.close()
			self.send(lines)
			time.sleep(1)
		self.send("grid")
		time.sleep(1)
		self.send(str(self.pages))
		time.sleep(1)
		self.send(str(self.rows))
		time.sleep(1)
		self.send(str(self.columns))
		print "DONE"
		
	def syncImage(self, id):
		print "SYNC IMAGE"
		self.send("image")
		time.sleep(1)
		self.send(str(id))
		time.sleep(1)
		file = open("Resources/"+str(id)+".png", "rb")
		lines = base64.b64encode(file.read())
		file.close()
		self.send(lines)
		print "DONE"
			
	def connect(self):
		server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		server.bind(ADDRESS)
		server.listen(1)
		while True:
			print "RESET"
			self.conn, address = server.accept()
			c = True
			print "accepted"

			while c:
				data = self.conn.recv(2048)
				c = self.grid(data)
			
	def grid(self, data):
		print "DATA:", data, "DATA END"
		try:
			self.run(int(data))
			return True
		except:
			return False
			
	def send(self, msg):
		self.conn.send(msg+"EOFEOFEOFEOFEOFEOFEOFEOFXXX")
	
	def onClose(self, event):
		self.Iconize()
		self.tbIcon = CustomTaskBarIcon(self)
		self.Hide()
			
def receiver(type):
	server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
	server.bind(ADDRESS)
	print type

	while True:
		data, address = server.recvfrom(4096)
		if data == "exit":
			return
		if type == "trackpad":
			threading.Thread(target=trackpad, args=(data,)).start()
		elif type == "keyboard":
			threading.Thread(target=keyboard, args=(data,)).start()
		elif type == "numpad":
			threading.Thread(target=numpad, args=(data,)).start()

def trackpad(data):
	print data
	
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
	print data
	pyautogui.press(data)
	
def numpad(data):
	print data
	pyautogui.press(data)


def main():
	app = wx.App()
	frame = MainFrame()
	app.MainLoop()
			

if __name__ == '__main__':
	main()
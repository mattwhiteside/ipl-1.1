/*
 * Copyright 2005-2008 Theseus Research Inc., All Rights Reserved.
 * 
 * This file may be used under the terms of the GNU General Public
 * License version 2.0 as published by the Free Software Foundation                                
 * and appearing in the file LICENSE.GPL included in the packaging of
 * this file. For commercial licensing contact info@vectaport.com
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND
 *
 */

/*
 * IplKit definitions
 */

#include <IplEdit/iplabout.h>
#include <IplEdit/iplcomps.h>
#include <IplEdit/iplkit.h>
#include <IplEdit/iplcmds.h>
#include <IplEdit/iplimport.h>
#if defined(ARCH_READY)
#include <IplServ/archcomps.h>
#endif
#include <IplServ/iplcomps.h>

#include <DrawServ/drawcomps.h>
#include <DrawServ/draweditor.h>
#include <DrawServ/drawserv.h>

#include <FrameUnidraw/frameeditor.h>

#include <GraphUnidraw/edgecomp.h>
#include <GraphUnidraw/nodecomp.h>
#include <GraphUnidraw/graphcmds.h>
#include <GraphUnidraw/graphcomp.h>
#include <GraphUnidraw/grapheditor.h>
#include <GraphUnidraw/graphimport.h>
#include <GraphUnidraw/graphkit.h>

#include <OverlayUnidraw/annotate.h>
#include <OverlayUnidraw/attrtool.h>
#include <OverlayUnidraw/ovabout.h>
#include <OverlayUnidraw/ovarrow.h>
#include <OverlayUnidraw/ovcamcmds.h>
#include <OverlayUnidraw/ovcmds.h>
#include <OverlayUnidraw/ovctrl.h>
#include <OverlayUnidraw/ovdoer.h>
#include <OverlayUnidraw/oved.h>
#include <OverlayUnidraw/ovellipse.h>
#include <OverlayUnidraw/ovexport.h>
#include <OverlayUnidraw/ovpolygon.h>
#include <OverlayUnidraw/ovprecise.h>
#include <OverlayUnidraw/ovprint.h>
#include <OverlayUnidraw/ovrect.h>
#include <OverlayUnidraw/ovtext.h>
#include <OverlayUnidraw/ovviewer.h>
#include <OverlayUnidraw/slctbyattr.h>

#include <UniIdraw/idarrows.h>
#include <UniIdraw/idkybd.h>

#include <Unidraw/Commands/align.h>
#include <Unidraw/Commands/transforms.h>
#include <Unidraw/Components/text.h>
#include <Unidraw/Graphic/ellipses.h>
#include <Unidraw/Graphic/picture.h>
#include <Unidraw/Graphic/polygons.h>
#include <Unidraw/Tools/connect.h>
#include <Unidraw/Tools/grcomptool.h>
#include <Unidraw/Tools/magnify.h>
#include <Unidraw/Tools/move.h>
#include <Unidraw/Tools/reshape.h>
#include <Unidraw/Tools/rotate.h>
#include <Unidraw/Tools/scale.h>
#include <Unidraw/Tools/select.h>
#include <Unidraw/Tools/stretch.h>
#include <Unidraw/catalog.h>
#include <Unidraw/ctrlinfo.h>
#include <Unidraw/keymap.h>
#include <Unidraw/kybd.h>
#include <Unidraw/unidraw.h>

#include <IVGlyph/figure.h>
#include <IVGlyph/importchooser.h>
#include <IVGlyph/toolbutton.h>
#include <IV-look/kit.h>
#include <InterViews/background.h>
#include <InterViews/brush.h>
#include <InterViews/layout.h>
#include <InterViews/patch.h>
#include <InterViews/session.h>
#include <InterViews/style.h>
#include <OS/math.h>

declareActionCallback(IplKit)
implementActionCallback(IplKit)

static const int unit = 15;
static const int xradius = 35;
static const int yradius = 20;

static int xClosed[] = { unit/5, unit, unit, unit*3/5, 0 };
static int yClosed[] = { 0, unit/5, unit*3/5, unit, unit*2/5 };
static Coord fxClosed[] = { unit/5, unit, unit, unit*3/5, 0 };
static Coord fyClosed[] = { 0, unit/5, unit*3/5, unit, unit*2/5 };
static const int nClosed = 5;

static int xOpen[] = { 0, unit/2, unit/2, unit };
static int yOpen[] = { 0, unit/4, unit*3/4, unit };
static Coord fxOpen[] = { 0, unit/2, unit/2, unit };
static Coord fyOpen[] = { 0, unit/4, unit*3/4, unit };
static const int nOpen = 4;

static Coord xhead[] = { unit, unit-4, unit-7 };
static Coord yhead[] = { unit, unit-7, unit-4 };

/******************************************************************************/

IplKit* IplKit::_iplkit = nil;

IplKit::IplKit()
: DrawKit()
{
}

IplKit::~IplKit() {
}

IplKit* IplKit::Instance() {
    if (!_iplkit)
	_iplkit = new IplKit();
    return _iplkit;
}

Glyph* IplKit::MakeToolbar() {
    WidgetKit& kit = *WidgetKit::instance();
    const LayoutKit& layout = *LayoutKit::instance();
    Style* s = kit.style();

    ToolButton* select;
    ToolButton* move;
    ToolButton* reshape;
    ToolButton* magnify;

    _toolbars = layout.deck(2);

    PolyGlyph* vb = layout.vbox();
    _toolbar_vbox = new Glyph*[2];
    _toolbar_vbox[0] = vb;

    _tg = new TelltaleGroup();

    Glyph* sel = kit.label("Select");
    Glyph* mov = kit.label("Move");
    Glyph* scl = kit.label("Scale");
    Glyph* str = kit.label("Stretch");
    Glyph* rot = kit.label("Rotate");
    Glyph* alt = kit.label("Alter");
    Glyph* mag = kit.label("Magnify");
    Glyph* txt = kit.label("Text");
    Glyph* glin = new Fig31Line(new Brush(0), kit.foreground(), nil,
				0, 0, unit, unit);
    Glyph* gmlin = new Fig31Polyline(new Brush(0), kit.foreground(), nil,
				     fxOpen, fyOpen, nOpen);
    Glyph* gospl = new Fig31Open_BSpline(new Brush(0), kit.foreground(), nil,
					 fxOpen, fyOpen, nOpen);
    Glyph* grect = new Fig31Rectangle(new Brush(0), kit.foreground(), nil,
				      0, 0, unit, unit*4/5);
    Glyph* gellp = new Fig31Ellipse(new Brush(0), kit.foreground(), nil,
				    0, 0, unit*2/3, unit*2/5);
    Glyph* gpoly = new Fig31Polygon(new Brush(0), kit.foreground(), nil,
				    fxClosed, fyClosed, nClosed);
    Glyph* gcspl = new Fig31Closed_BSpline(new Brush(0), kit.foreground(), nil,
					   fxClosed, fyClosed, nClosed);
    Glyph* anno = kit.label("Annotate");
    Glyph* attr = kit.label("Attribute");
    Glyph* gedge = layout.overlay(
	new Fig31Line(new Brush(0), kit.foreground(), nil,
		      0, 0, unit, unit),
	new Fig31Polygon(new Brush(0), kit.foreground(), kit.foreground(),
			 xhead, yhead, 3)
    );
    Glyph* gnod1 = layout.overlay(
	new Fig31Ellipse(new Brush(0), kit.foreground(), nil,
			 0, 0, unit, unit*3/5),
	layout.center(kit.label("___")));
    Glyph* gnod2 = layout.overlay(
	new Fig31Ellipse(new Brush(0), kit.foreground(), nil,
			 0, 0, unit, unit*3/5),
	layout.center(kit.label("abc")));
    Glyph* gnod3 = layout.overlay(
	new Fig31Ellipse(new Brush(0), kit.foreground(), nil,
			 0, 0, unit, unit*3/5),
	layout.center(kit.label("arbit")));
    Glyph* gnod4 = layout.overlay(
	new Fig31Rectangle(new Brush(0), kit.foreground(), nil,
			 -unit, -unit, unit, unit),
	layout.center(kit.label("instr")));
    Glyph* gnod5 = layout.overlay(
	new Fig31Rectangle(new Brush(0), kit.foreground(), nil,
			 -unit, -unit, unit, unit),
	layout.center(kit.label("ALU")));
    Glyph* gnod6 = layout.overlay(
	new Fig31Rectangle(new Brush(0), kit.foreground(), nil,
			 -unit, -unit, unit, unit),
	layout.center(kit.label("IAD")));
    Glyph* gnod7 = layout.overlay(
	new Fig31Rectangle(new Brush(0), kit.foreground(), nil,
			 -unit, -unit, unit, unit),
	layout.center(kit.label("OAD")));
    Glyph* gnod8 = layout.overlay(
	new Fig31Ellipse(new Brush(0), kit.foreground(), kit.foreground(),
			 0, 0, 3, 3),
	layout.center(kit.label(".")));
    Glyph* gnod9 = layout.overlay(
	new Fig31Rectangle(new Brush(0), kit.foreground(), nil,
			 -unit, -unit, unit, unit),
	layout.center(kit.label("MEM")));

    Coord maxwidth = 0;
    Requisition req;
    maxwidth = Math::max((sel->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((mov->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((scl->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((str->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((rot->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((alt->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((mag->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((txt->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((glin->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gmlin->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gospl->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((grect->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gellp->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gpoly->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gcspl->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((anno->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gedge->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gnod1->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gnod2->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gnod3->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gnod4->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gnod5->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gnod6->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gnod7->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gnod8->request(req), req.x_requirement().natural()),
			 maxwidth);
    maxwidth = Math::max((gnod9->request(req), req.x_requirement().natural()),
			 maxwidth);

    vb->append(select = MakeTool(new SelectTool(new ControlInfo("Select", KLBL_SELECT, CODE_SELECT)),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(sel)),
			_tg, _ed->MouseDocObservable(), mouse_sel));
    vb->append(move = MakeTool(new MoveTool(new ControlInfo("Move", KLBL_MOVE, CODE_MOVE)),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(mov)),
			_tg, _ed->MouseDocObservable(), mouse_mov));
    vb->append(MakeTool(new ScaleTool(new ControlInfo("Scale", KLBL_SCALE, CODE_SCALE)),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(scl)), _tg, _ed->MouseDocObservable(), mouse_scl));
    vb->append(MakeTool(new StretchTool(new ControlInfo("Stretch", KLBL_STRETCH,CODE_STRETCH)),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(str)), _tg, _ed->MouseDocObservable(), mouse_str));
    vb->append(MakeTool(new RotateTool(new ControlInfo("Rotate", KLBL_ROTATE, CODE_ROTATE)),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(rot)), _tg, _ed->MouseDocObservable(), mouse_rot));
    vb->append(reshape = MakeTool(new ReshapeTool(new ControlInfo("Alter", KLBL_RESHAPE, CODE_RESHAPE)),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(alt)), _tg, _ed->MouseDocObservable(), mouse_alt));
    vb->append(magnify = MakeTool(new MagnifyTool(new ControlInfo("Magnify", KLBL_MAGNIFY,CODE_MAGNIFY)),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(mag)), _tg, _ed->MouseDocObservable(), mouse_mag));
    TextGraphic* text = new TextGraphic("Text", stdgraphic);
    TextOvComp* textComp = new TextOvComp(text);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo("Text", KLBL_TEXT, CODE_TEXT), textComp),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(txt)), _tg, _ed->MouseDocObservable(), mouse_txt));
    ArrowLine* line = new ArrowLine(
	0, 0, unit, unit, false, false, 1., stdgraphic
    );
    ArrowLineOvComp* arrowLineComp = new ArrowLineOvComp(line);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(arrowLineComp, KLBL_LINE, CODE_LINE), arrowLineComp),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(glin)),
			_tg, _ed->MouseDocObservable(), mouse_lin));
    ArrowMultiLine* ml = new ArrowMultiLine(
        xOpen, yOpen, nOpen, false, false, 1., stdgraphic
    );
    ml->SetPattern(psnonepat);
    ArrowMultiLineOvComp* mlComp = new ArrowMultiLineOvComp(ml);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(mlComp, KLBL_MULTILINE, CODE_MULTILINE), mlComp),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gmlin)),
			_tg, _ed->MouseDocObservable(), mouse_mlin));
    ArrowOpenBSpline* spl = new ArrowOpenBSpline(
        xOpen, yOpen, nOpen, false, false, 1., stdgraphic
    );
    spl->SetPattern(psnonepat);
    ArrowSplineOvComp* splComp = new ArrowSplineOvComp(spl);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(splComp, KLBL_SPLINE, CODE_SPLINE), splComp),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gospl)),
			_tg, _ed->MouseDocObservable(), mouse_ospl));
    SF_Rect* rect = new SF_Rect(0, 0, unit, unit*4/5, stdgraphic);
    rect->SetPattern(psnonepat);
    RectOvComp* rectComp = new RectOvComp(rect);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(rectComp, KLBL_RECT, CODE_RECT), rectComp),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(grect)),
			_tg, _ed->MouseDocObservable(), mouse_rect));
    SF_Ellipse* ellipse = new SF_Ellipse(0, 0, unit*2/3, unit*2/5, stdgraphic);
    ellipse->SetPattern(psnonepat);
    EllipseOvComp* ellipseComp = new EllipseOvComp(ellipse);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(ellipseComp, KLBL_ELLIPSE, CODE_ELLIPSE), ellipseComp),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gellp)),
			_tg, _ed->MouseDocObservable(), mouse_ellp));
    SF_Polygon* polygon = new SF_Polygon(xClosed, yClosed, nClosed,stdgraphic);
    polygon->SetPattern(psnonepat);
    PolygonOvComp* polygonComp = new PolygonOvComp(polygon);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(polygonComp, KLBL_POLY, CODE_POLY), polygonComp),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gpoly)),
			_tg, _ed->MouseDocObservable(), mouse_poly));
    SFH_ClosedBSpline* cspline = new SFH_ClosedBSpline(
        xClosed, yClosed, nClosed, stdgraphic
    );
    cspline->SetPattern(psnonepat);
    ClosedSplineOvComp* csplineComp = new ClosedSplineOvComp(cspline);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(csplineComp, KLBL_CSPLINE,CODE_CSPLINE), csplineComp),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gcspl)),
			_tg, _ed->MouseDocObservable(), mouse_cspl));
    vb->append(MakeTool(new AnnotateTool(new ControlInfo("Annotate", "A", "A")),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(anno)), _tg, _ed->MouseDocObservable(), mouse_anno));
    vb->append(MakeTool(new AttributeTool(new ControlInfo("Attribute", "T", "T")),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(attr)), _tg, _ed->MouseDocObservable(), mouse_attr));

    _toolbars->append(vb);
    vb = layout.vbox();
    _toolbar_vbox[1] = vb;
    vb->append(select);
    vb->append(move);
    vb->append(reshape);
    vb->append(magnify);
    ArrowLine* aline = new ArrowLine(
	0, 0, unit, unit, true, false, 1., stdgraphic
    );

    EdgeComp* protoedge = new ConnComp(aline);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(protoedge, "E","E"),
					    protoedge),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gedge)), _tg, _ed->MouseDocObservable(), GraphKit::mouse_edge));

    SF_Ellipse* nellipse = new SF_Ellipse(0, 0, xradius, yradius, stdgraphic);
    nellipse->SetPattern(psnonepat);
    TextGraphic* ntext = new TextGraphic("", stdgraphic);
    TextGraphic* ntext2 = new TextGraphic("", stdgraphic);
    PipeComp* protonode = new PipeComp(nellipse, ntext, ntext2, nil);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(protonode, "N","N"),
					    protonode),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gnod1)), _tg, _ed->MouseDocObservable(), GraphKit::mouse_node));


    SF_Ellipse* nellipse2 = new SF_Ellipse(0, 0, xradius, yradius, stdgraphic);
    nellipse2->SetPattern(psnonepat);
    TextGraphic* ntext3 = new TextGraphic("", stdgraphic);
    TextGraphic* ntext4 = new TextGraphic("", stdgraphic);
    PipeComp* protonode2 = new PipeComp(nellipse2, ntext3, ntext4, true);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(protonode2, "L","L"),
					    protonode2),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gnod2)), _tg, _ed->MouseDocObservable(), GraphKit::mouse_lnode));

    SF_Ellipse* nellipse3 = new SF_Ellipse(0, 0, xradius, yradius, stdgraphic);
    nellipse3->SetPattern(psnonepat);
    TextGraphic* ntext5 = new TextGraphic("", stdgraphic);
    TextGraphic* ntext6 = new TextGraphic("", stdgraphic);
    ArbiterComp* protonode3 = new ArbiterComp(nellipse3, ntext5, ntext6, true);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(protonode3, "M","M"),
					    protonode3),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gnod3)), _tg, _ed->MouseDocObservable(), GraphKit::mouse_lnode));

    SF_Rect* nrect1 = new SF_Rect(0, 0, 64, 64, stdgraphic);
    nrect1->SetPattern(psnonepat);
    TextGraphic* ntext7 = new TextGraphic("", stdgraphic);
    TextGraphic* ntext8 = new TextGraphic("", stdgraphic);
    TextGraphic* ntext9 = new TextGraphic("", stdgraphic);
    Picture* picture1 = new Picture(stdgraphic);
    picture1->Append(nrect1, ntext7, ntext8, ntext9);
    Picture* subpic0 = new Picture(stdgraphic);
    Picture* subpic1 = new Picture(stdgraphic);
    Picture* subpic2 = new Picture(stdgraphic);
    picture1->Append(subpic0, subpic1, subpic2);
    InvoComp* protonode4 = new InvoComp(picture1);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(protonode4, "I","I"),
					    protonode4),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gnod4)), _tg, _ed->MouseDocObservable(), GraphKit::mouse_lnode));

    SF_Ellipse* nellipse4 = new SF_Ellipse(0, 0, 3, 3, stdgraphic);
    nellipse4->SetPattern(pssolid);
    TextGraphic* ntext19 = new TextGraphic("", stdgraphic);
    TextGraphic* ntext20 = new TextGraphic("", stdgraphic);
    ForkComp* protonode8 = new ForkComp(nellipse4, ntext19, ntext20, nil);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(protonode8, "",""),
					    protonode8),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gnod8)), _tg, _ed->MouseDocObservable(), GraphKit::mouse_node));


#if defined(ARCH_READY)
    Catalog* catalog = unidraw->GetCatalog();
    const char* mem = catalog->GetAttribute("mem");
    int memflag = mem && (!strcmp(mem, "true") || !strcmp(mem, "TRUE"));
    if (memflag) {

    SF_Rect* nrect5 = new SF_Rect(0, 0, 64, 64, stdgraphic);
    nrect5->SetPattern(psnonepat);
    TextGraphic* ntext21 = new TextGraphic("", stdgraphic);
    TextGraphic* ntext22 = new TextGraphic("", stdgraphic);
    TextGraphic* ntext23 = new TextGraphic("", stdgraphic);
    Picture* picture5 = new Picture(stdgraphic);
    picture5->Append(nrect5, ntext21, ntext22, ntext23);
    Picture* subpic12 = new Picture(stdgraphic);
    Picture* subpic13 = new Picture(stdgraphic);
    Picture* subpic14 = new Picture(stdgraphic);
    picture5->Append(subpic12, subpic13, subpic14);
    MemComp* protonode9 = new MemComp(picture5);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(protonode9, "",""),
					    protonode9),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gnod9)), _tg, _ed->MouseDocObservable(), GraphKit::mouse_lnode));
    }
#endif

#if defined(ARCH_READY)
    const char* arch = catalog->GetAttribute("arch");
    int archflag = arch && (!strcmp(arch, "true") || !strcmp(arch, "TRUE"));
    if (archflag) {

    SF_Rect* nrect2 = new SF_Rect(0, 0, 64, 64, stdgraphic);
    nrect2->SetPattern(psnonepat);
    TextGraphic* ntext10 = new TextGraphic("", stdgraphic);
    TextGraphic* ntext11 = new TextGraphic("", stdgraphic);
    TextGraphic* ntext12 = new TextGraphic("", stdgraphic);
    Picture* picture2 = new Picture(stdgraphic);
    picture2->Append(nrect2, ntext10, ntext11, ntext12);
    Picture* subpic3 = new Picture(stdgraphic);
    Picture* subpic4 = new Picture(stdgraphic);
    Picture* subpic5 = new Picture(stdgraphic);
    picture2->Append(subpic3, subpic4, subpic5);
    AluComp* protonode5 = new AluComp(picture2);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(protonode5, "A","A"),
					    protonode5),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gnod5)), _tg, _ed->MouseDocObservable(), GraphKit::mouse_lnode));

    SF_Rect* nrect3 = new SF_Rect(0, 0, 64, 64, stdgraphic);
    nrect3->SetPattern(psnonepat);
    TextGraphic* ntext13 = new TextGraphic("", stdgraphic);
    TextGraphic* ntext14 = new TextGraphic("", stdgraphic);
    TextGraphic* ntext15 = new TextGraphic("", stdgraphic);
    Picture* picture3 = new Picture(stdgraphic);
    picture3->Append(nrect3, ntext13, ntext14, ntext15);
    Picture* subpic6 = new Picture(stdgraphic);
    Picture* subpic7 = new Picture(stdgraphic);
    Picture* subpic8 = new Picture(stdgraphic);
    picture3->Append(subpic6, subpic7, subpic8);
    IadComp* protonode6 = new IadComp(picture3);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(protonode6, "",""),
					    protonode6),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gnod6)), _tg, _ed->MouseDocObservable(), GraphKit::mouse_lnode));

    SF_Rect* nrect4 = new SF_Rect(0, 0, 64, 128, stdgraphic);
    nrect4->SetPattern(psnonepat);
    TextGraphic* ntext16 = new TextGraphic("", stdgraphic);
    TextGraphic* ntext17 = new TextGraphic("", stdgraphic);
    TextGraphic* ntext18 = new TextGraphic("", stdgraphic);
    Picture* picture4 = new Picture(stdgraphic);
    picture4->Append(nrect4, ntext16, ntext17, ntext18);
    Picture* subpic9 = new Picture(stdgraphic);
    Picture* subpic10 = new Picture(stdgraphic);
    Picture* subpic11 = new Picture(stdgraphic);
    picture4->Append(subpic9, subpic10, subpic11);
    OadComp* protonode7 = new OadComp(picture4);
    vb->append(MakeTool(new GraphicCompTool(new ControlInfo(protonode7, "",""),
					    protonode7),
			layout.overlay(layout.hcenter(layout.hspace(maxwidth)),
				       layout.hcenter(gnod7)), _tg, _ed->MouseDocObservable(), GraphKit::mouse_lnode));
    } /* done with if (archflag) */
#endif

    _toolbars->append(vb);

    _toolbars->flip_to(1);
    _toolbar = new Patch(_toolbars);

    return layout.hbox(
	layout.vflexible(
	    layout.vnatural(
		new Background(
		    layout.vcenter(
			_toolbar
		    ),
		    unidraw->GetCatalog()->FindColor("#aaaaaa")
		),
		550
	    )
	)
    );
}

MenuItem * IplKit::MakeFileMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    
    MenuItem *mbi = kit.menubar_item(kit.label("File"));
    mbi->menu(kit.pulldown());
    MakeMenu(mbi, new IplAboutCmd(new ControlInfo("About ipledit", KLBL_ABOUT, CODE_ABOUT)),
	     "About ipledit   ");
    MakeMenu(mbi, new OvNewCompCmd(new ControlInfo("New", KLBL_NEWCOMP, CODE_NEWCOMP),
				 new IplIdrawComp),
	     "New   ");
    MakeMenu(mbi, new OvRevertCmd(new ControlInfo("Revert", KLBL_REVERT, CODE_REVERT)),
	     "Revert   ");
    MakeMenu(mbi, new OvOpenCmd(new ControlInfo("Open...", KLBL_VIEWCOMP, CODE_VIEWCOMP)),
	     "Open...   ");
    MakeMenu(mbi, new OvSaveCompCmd(new ControlInfo("Save", KLBL_SAVECOMP, CODE_SAVECOMP)),
	     "Save   ");
    MakeMenu(mbi, new OvSaveCompAsCmd(new ControlInfo("Save As...",
						      KLBL_SAVECOMPAS, CODE_SAVECOMPAS)),
	     "Save As...   ");
    MakeMenu(mbi, new OvPrintCmd(new ControlInfo("Print...", KLBL_PRINT, CODE_PRINT)),
	     "Print...   ");
    Style* style;
    style = new Style(Session::instance()->style());
    style->attribute("subcaption", "Import graphic from file:");
    style->attribute("open", "Import");
    ImportChooser* ichooser = new ImportChooser(".", WidgetKit::instance(), style, nil,
						true /* centered */,
						true /* by pathname */);
    MakeMenu(mbi, new IplImportCmd(new ControlInfo("Import Drawing...",
						  KLBL_IMPORT,
						  CODE_IMPORT),
				   ichooser),
	     "Import Drawing...   ");
    MakeMenu(mbi, new OvExportCmd(new ControlInfo("Export Graphic...",
						  "^X", "\030")),
	     "Export Graphic...   ");
    mbi->menu()->append_item(kit.menu_item_separator());
    MakeMenu(mbi, new OvQuitCmd(new ControlInfo("Quit", KLBL_QUIT, CODE_QUIT)),
	     "Quit   ");
    return mbi;
}

MenuItem* IplKit::MakeEditMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    
    MenuItem *mbi = kit.menubar_item(kit.label("Edit"));
    mbi->menu(kit.pulldown());

    MakeMenu(mbi, new UndoCmd(new ControlInfo("Undo", KLBL_UNDO, CODE_UNDO)),
	     "Undo   ");
    MakeMenu(mbi, new RedoCmd(new ControlInfo("Redo", KLBL_REDO, CODE_REDO)),
	     "Redo   ");
    MakeMenu(mbi, new GraphCutCmd(new ControlInfo("Cut", KLBL_CUT, CODE_CUT)),
	     "Cut   "); 
    MakeMenu(mbi, new GraphCopyCmd(new ControlInfo("Copy", KLBL_COPY, CODE_COPY)),
	     "Copy   ");
    MakeMenu(mbi, new IplPasteCmd(new ControlInfo("Paste", KLBL_PASTE, CODE_PASTE)),
	     "Paste   ");
    MakeMenu(mbi, new IplDupCmd(new ControlInfo("Duplicate", KLBL_DUP, CODE_DUP)),
	     "Duplicate   ");
    MakeMenu(mbi, new GraphDeleteCmd(new ControlInfo("Delete", KLBL_DEL, CODE_DEL)),
	     "Delete   ");
    MakeMenu(mbi, new OvSlctAllCmd(new ControlInfo("Select All", KLBL_SLCTALL, CODE_SLCTALL)),
	     "Select All   ");
    MakeMenu(mbi, new SlctByAttrCmd(new ControlInfo("Select by Attribute", "$", "$")),
	     "Select by Attribute   ");
    mbi->menu()->append_item(kit.menu_item_separator());
    MakeMenu(mbi, new ScaleCmd(new ControlInfo("Flip Horizontal",
				       KLBL_HFLIP, CODE_HFLIP),
		       -1.0, 1.0),
	     "Flip Horizontal   ");
    MakeMenu(mbi, new ScaleCmd(new ControlInfo("Flip Vertical",
				       KLBL_VFLIP, CODE_VFLIP),
		       1.0, -1.0),
	     "Flip Vertical   ");
    MakeMenu(mbi, new RotateCmd(new ControlInfo("90 Clockwise", KLBL_CW90, CODE_CW90),
			-90.0),
	     "90 Clockwise   ");
    MakeMenu(mbi, new RotateCmd(new ControlInfo("90 CounterCW", KLBL_CCW90, CODE_CCW90),
			90.0),
	     "90 CounterCW   ");
    mbi->menu()->append_item(kit.menu_item_separator());
    MakeMenu(mbi, new OvPreciseMoveCmd(new ControlInfo("Precise Move",
					     KLBL_PMOVE, CODE_PMOVE)),
	     "Precise Move   ");
    MakeMenu(mbi, new OvPreciseScaleCmd(new ControlInfo("Precise Scale",
					      KLBL_PSCALE, CODE_PSCALE)),
	     "Precise Scale   ");
    MakeMenu(mbi, new OvPreciseRotateCmd(new ControlInfo("Precise Rotate",
					       KLBL_PROTATE, CODE_PROTATE)),
	     "Precise Rotate   ");

    return mbi;
}

#undef Center
#define Center iv2_6_Center

MenuItem* IplKit::MakeAlignMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    MenuItem* mbi = kit.menubar_item(kit.label("Align"));
    mbi->menu(kit.pulldown());

    MakeMenu(mbi, new SquareCmd(new ControlInfo("Square Wires", "", "")), "Square Wires ");
    MakeMenu(mbi, new ReorientTextCmd(new ControlInfo("Reorient Text", "", "")), "Reorient Text");
    mbi->menu()->append_item(kit.menu_item_separator());
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Left Sides",
				       KLBL_ALGNLEFT,
				       CODE_ALGNLEFT), Left, Left),
	     "Left Sides   ");
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Right Sides",
				       KLBL_ALGNRIGHT,
				       CODE_ALGNRIGHT), Right, Right),
	     "Right Sides   ");
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Tops",
				       KLBL_ALGNTOP,
				       CODE_ALGNTOP), Top, Top),
	     "Tops   ");
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Bottoms",
				       KLBL_ALGNBOT,
				       CODE_ALGNBOT), Bottom, Bottom),
	     "Bottoms   ");
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Horiz Centers",
				       KLBL_ALGNHCTR,
				       CODE_ALGNHCTR), HorizCenter, HorizCenter),
	     "Horiz Centers   ");
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Vert Centers",
				       KLBL_ALGNVCTR,
				       CODE_ALGNVCTR), VertCenter, VertCenter),
	     "Vert Centers   ");
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Centers",
				       KLBL_ALGNCTR,
				       CODE_ALGNCTR), Center, Center),
	     "Centers   ");
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Abut Left",
				       KLBL_ABUTLEFT,
				       CODE_ABUTLEFT), Left, Right),
	     "Abut Left   ");
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Abut Right",
				       KLBL_ABUTRIGHT,
				       CODE_ABUTRIGHT), Right, Left),
	     "Abut Right   ");
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Abut Up",
				       KLBL_ABUTUP,
				       CODE_ABUTUP), Top, Bottom),
	     "Abut Up   ");
    MakeMenu(mbi, new AlignCmd(new ControlInfo("Abut Down",
				       KLBL_ABUTDOWN,
				       CODE_ABUTDOWN), Bottom, Top),
	     "Abut Down   ");
    mbi->menu()->append_item(kit.menu_item_separator());
    MakeMenu(mbi, new AlignToGridCmd(new ControlInfo("Align to Grid",
					     KLBL_ALGNTOGRID,
					     CODE_ALGNTOGRID)),
	     "Align to Grid   ");

    return mbi;
}

MenuItem* IplKit::MakeFrameMenu() {
    LayoutKit& lk = *LayoutKit::instance();
    WidgetKit& kit = *WidgetKit::instance();
    
    MenuItem *mbi = kit.menubar_item(kit.label("Frame"));
    mbi->menu(kit.pulldown());

    MoveFrameCmd::default_instance
      (new MoveFrameCmd(new ControlInfo("Move Forward","^F",""), +1));
    MakeMenu(mbi, MoveFrameCmd::default_instance(),
	     "Move Forward   ");

    MakeMenu(mbi, new MoveFrameCmd(new ControlInfo("Move Backward","^B",""), -1),
	     "Move Backward   ");
    MakeMenu(mbi, new FrameBeginCmd(new ControlInfo("Goto First Frame")),
	     "Goto First Frame");
    MakeMenu(mbi, new FrameEndCmd(new ControlInfo("Goto Last Frame")),
	     "Goto Last Frame ");
    mbi->menu()->append_item(kit.menu_item_separator());
    MakeMenu(mbi, new CreateMoveFrameCmd(new ControlInfo("New Forward","F","F")),
	     "New Forward    ");
    MakeMenu(mbi, new CreateMoveFrameCmd(new ControlInfo("New Backward","B","B"), false),
	     "New Backward   ");
    MakeMenu(mbi, new CopyMoveIplFrameCmd(new ControlInfo("Copy Forward","X","X")),
	     "Copy Forward   ");
    MakeMenu(mbi, new CopyMoveIplFrameCmd(new ControlInfo("Copy Backward","Y","Y"), false),
	     "Copy Backward  ");
    MakeMenu(mbi, new DeleteFrameCmd(new ControlInfo("Delete","D","D")),
	     "Delete  ");
    mbi->menu()->append_item(kit.menu_item_separator());
    MakeMenu(mbi, new ShowOtherFrameCmd(new ControlInfo("Show Prev Frame","",""), -1),
	     "Show Prev Frame");
    MakeMenu(mbi, new ShowOtherFrameCmd(new ControlInfo("Hide Prev Frame","",""), 0),
	     "Hide Prev Frame");

    MenuItem* menu_item;
    menu_item = kit.menu_item(kit.label("Enable Looping"));
    menu_item->action
      (new ActionCallback(MoveFrameCmd)
       (MoveFrameCmd::default_instance(), &MoveFrameCmd::set_wraparound));
    mbi->menu()->append_item(menu_item);

    menu_item = kit.menu_item(kit.label("Disable Looping"));
    menu_item->action
      (new ActionCallback(MoveFrameCmd)
       (MoveFrameCmd::default_instance(), &MoveFrameCmd::clr_wraparound));
    mbi->menu()->append_item(menu_item);

#if 0
    MakeMenu(mbi, new AutoNewFrameCmd(new ControlInfo("Toggle Auto New Frame",
						      "","")),
	     "Toggle Auto New Frame");
#else
    menu_item = kit.check_menu_item(kit.label("Auto New Frame"));
    menu_item->state()->set(TelltaleState::is_chosen, ((FrameEditor*)GetEditor())->AutoNewFrame());
    ((FrameEditor*)GetEditor())->_autonewframe_tts = menu_item->state();
    AutoNewFrameCmd::default_instance(new AutoNewFrameCmd(GetEditor()));
    menu_item->action
      (new ActionCallback(AutoNewFrameCmd)
       (AutoNewFrameCmd::default_instance(), &AutoNewFrameCmd::Execute));
    mbi->menu()->append_item(menu_item);
#endif
    return mbi;
}

Glyph* IplKit::appicon() {
  const LayoutKit& lk = *LayoutKit::instance();
  WidgetKit& wk = *WidgetKit::instance();
  return lk.vbox(lk.hcenter(wk.label("ipledit")), 
		 lk.vspace(20),
		 lk.hcenter(wk.label("type help for list of")),
		 lk.hcenter(wk.label("keyboard commands"))
		 );
}


MenuItem * IplKit::MakeViewersMenu() {
    return nil;
}
